#ifndef INCLUDE_bot_lib_handler
#define INCLUDE_bot_lib_handler

#include "bot_lib/meta.hpp"
#include "bot_lib/state.hpp"
#include "bot_lib/state_storage/common.hpp"

#include <tgbot/Bot.h>
#include <tgbot/types/Message.h>

#include <concepts>
#include <tuple>
#include <type_traits>

namespace tg_stater {

namespace detail {

template <typename ReturnT_, typename... Args>
struct general_function_traits {
    using ReturnT = ReturnT_;

    constexpr static std::size_t arg_count = sizeof...(Args);

    template <std::size_t I>
    using ArgT = std::tuple_element<I, std::tuple<Args...>>::type;
};

template <typename F>
struct function_traits;

template <typename F>
    requires requires { &F::operator(); }
struct function_traits<F> : function_traits<decltype(&F::operator())> {};

// for non-static non-mutable lambdas
template <typename T, typename ReturnT, typename... Args>
struct function_traits<ReturnT (T::*)(Args...) const> : general_function_traits<ReturnT, Args...> {};

// for static lambdas and function pointers
template <typename ReturnT, typename... Args>
struct function_traits<ReturnT (*)(Args...)> : general_function_traits<ReturnT, Args...> {};

// for function types
template <typename ReturnT, typename... Args>
struct function_traits<ReturnT(Args...)> : function_traits<ReturnT (*)(Args...)> {};

} // namespace detail

namespace concepts {

template <typename F, typename StateT, typename StateProxyT>
concept EnumStateHandlerF = std::invocable<F, StateT&, const TgBot::Message&, TgBot::Bot&, const StateProxyT&>;

template <typename F, typename StateProxyT>
concept NoStateHandlerF = std::invocable<F, const TgBot::Message&, TgBot::Bot&, const StateProxyT&>;

template <typename F, typename StateProxyT>
concept AnyStateHandlerF = NoStateHandlerF<F, StateProxyT>;

} // namespace concepts

template <EnumState S, typename StateProxyT, concepts::EnumStateHandlerF<S, StateProxyT> F>
struct EnumStateHandler {
    S state;
    F handler;
};
template <State S, typename F>
EnumStateHandler(S, const F&)
    -> EnumStateHandler<S, std::remove_cvref_t<typename detail::function_traits<F>::template ArgT<3>>, F>;

template <typename StateProxyT, concepts::NoStateHandlerF<StateProxyT> F>
struct NoStateHandler {
    F handler;
};
template <typename F>
NoStateHandler(const F&)
    -> NoStateHandler<std::remove_cvref_t<typename detail::function_traits<F>::template ArgT<2>>, F>;

template <typename StateProxyT, concepts::AnyStateHandlerF<StateProxyT> F>
struct AnyStateHandler {
    F handler;
};
template <typename F>
AnyStateHandler(const F&)
    -> AnyStateHandler<std::remove_cvref_t<typename detail::function_traits<F>::template ArgT<2>>, F>;

namespace concepts {

template <typename T, typename StateT, typename StateStorageT>
concept EnumStateHandler =
    State<StateT> && StateStorage<StateStorageT, StateT> &&
    meta::is_of_template<meta::Curry<EnumStateHandler, StateT, StateProxy<StateT, StateStorageT>>::template type, T>;

template <typename T, typename StateT, typename StateStorageT>
concept NoStateHandler =
    State<StateT> && StateStorage<StateStorageT, StateT> &&
    meta::is_of_template<meta::Curry<NoStateHandler, StateProxy<StateT, StateStorageT>>::template type, T>;

template <typename T, typename StateT, typename StateStorageT>
concept AnyStateHandler =
    State<StateT> && StateStorage<StateStorageT, StateT> &&
    meta::is_of_template<meta::Curry<AnyStateHandler, StateProxy<StateT, StateStorageT>>::template type, T>;

template <typename T, typename StateT, typename StateStorageT>
concept AnyHandler = EnumStateHandler<T, StateT, StateStorageT> || NoStateHandler<T, StateT, StateStorageT> ||
                     AnyStateHandler<T, StateT, StateStorageT>;

} // namespace concepts

} // namespace tg_stater
#endif // INCLUDE_bot_lib_handler
