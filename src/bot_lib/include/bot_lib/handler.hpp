#ifndef INCLUDE_bot_lib_handler
#define INCLUDE_bot_lib_handler

#include "bot_lib/meta.hpp"
#include "bot_lib/state.hpp"
#include "bot_lib/state_storage/common.hpp"

#include <concepts>
#include <tuple>
#include <type_traits>

namespace TgBot {
class Api;
class Message;
} // namespace TgBot

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
concept EnumStateHandlerF = std::invocable<F, StateT&, const TgBot::Message&, const TgBot::Api&, const StateProxyT&>;

// StateT here is a specific expected part of std::variant
template <typename F, typename StateT, typename StateProxyT>
concept VariantStateHandlerF = std::invocable<F, StateT&, const TgBot::Message&, const TgBot::Api&, const StateProxyT&>;

template <typename F, typename StateProxyT>
concept NoStateHandlerF = std::invocable<F, const TgBot::Message&, const TgBot::Api&, const StateProxyT&>;

template <typename F, typename StateProxyT>
concept AnyStateHandlerF = NoStateHandlerF<F, StateProxyT>;

} // namespace concepts

template <concepts::EnumState S, typename StateProxyT, concepts::EnumStateHandlerF<S, StateProxyT> F>
struct EnumStateHandler {
    S state;
    F handler;
};
template <concepts::EnumState S, typename F>
EnumStateHandler(S, const F&)
    -> EnumStateHandler<S, std::remove_cvref_t<typename detail::function_traits<F>::template ArgT<3>>, F>;

template <typename S, typename StateProxyT, concepts::VariantStateHandlerF<S, StateProxyT> F>
struct VariantStateHandler {
    using State = S;
    F handler;
};
template <typename F>
VariantStateHandler(const F&)
    -> VariantStateHandler<std::remove_cvref_t<typename detail::function_traits<F>::template ArgT<0>>,
                           std::remove_cvref_t<typename detail::function_traits<F>::template ArgT<3>>,
                           F>;

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
    EnumState<StateT> && StateStorage<StateStorageT, StateT> &&
    meta::IsOfTemplate<meta::Curry<EnumStateHandler, StateT, StateProxy<StateStorageT>>::template type, T>;

namespace detail {

template <typename T, typename StateStorageT>
struct CheckVariantStateHandler : std::false_type {};

template <typename S, typename StateStorageT, typename F>
    requires meta::IsPartOfVariant<S, typename StateStorageT::StateT>
struct CheckVariantStateHandler<VariantStateHandler<S, StateProxy<StateStorageT>, F>, StateStorageT> : std::true_type {
};

} // namespace detail

template <typename T, typename StateT, typename StateStorageT>
concept VariantStateHandler = VariantState<StateT> && StateStorage<StateStorageT, StateT> &&
                              detail::CheckVariantStateHandler<T, StateStorageT>::value;

template <typename T, typename StateT, typename StateStorageT>
concept NoStateHandler = State<StateT> && StateStorage<StateStorageT, StateT> &&
                         meta::IsOfTemplate<meta::Curry<NoStateHandler, StateProxy<StateStorageT>>::template type, T>;

template <typename T, typename StateT, typename StateStorageT>
concept AnyStateHandler = State<StateT> && StateStorage<StateStorageT, StateT> &&
                          meta::IsOfTemplate<meta::Curry<AnyStateHandler, StateProxy<StateStorageT>>::template type, T>;

template <typename T, typename StateT, typename StateStorageT>
concept AnyEnumStaterHandler = EnumStateHandler<T, StateT, StateStorageT> || NoStateHandler<T, StateT, StateStorageT> ||
                               AnyStateHandler<T, StateT, StateStorageT>;

template <typename T, typename StateT, typename StateStorageT>
concept AnyVariantStaterHandler = VariantStateHandler<T, StateT, StateStorageT> ||
                                  NoStateHandler<T, StateT, StateStorageT> || AnyStateHandler<T, StateT, StateStorageT>;

template <typename StateT, typename StateStorageT, auto... HandlerMetas>
concept CheckEnumStaterHandlers =
    EnumState<StateT> && (concepts::AnyEnumStaterHandler<decltype(HandlerMetas), StateT, StateStorageT> && ...);

template <typename StateT, typename StateStorageT, auto... HandlerMetas>
concept CheckVariantStaterHandlers =
    VariantState<StateT> && (concepts::AnyVariantStaterHandler<decltype(HandlerMetas), StateT, StateStorageT> && ...);

template <typename HandlerMeta, typename State, typename StateT, typename StateStorageT>
concept BelongsToStateHandler =
    VariantStateHandler<HandlerMeta, StateT, StateStorageT> && std::same_as<typename HandlerMeta::State, State>;

} // namespace concepts

} // namespace tg_stater
#endif // INCLUDE_bot_lib_handler
