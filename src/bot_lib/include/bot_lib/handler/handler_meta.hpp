#ifndef INCLUDE_bot_lib_handler_handler_meta
#define INCLUDE_bot_lib_handler_handler_meta

#include "bot_lib/handler/events.hpp"
#include "bot_lib/handler/type.hpp"
#include "bot_lib/meta.hpp"
#include "bot_lib/state.hpp"
#include "bot_lib/state_storage/common.hpp"

#include <concepts>
#include <tuple>
#include <type_traits>

namespace tg_stater {

namespace detail {

template <typename F>
    requires meta::is_of_template<std::remove_cvref_t<typename meta::function_traits<F>::template ArgT<1>>, StateProxy>
using parse_handler_proxy = std::remove_cvref_t<typename meta::function_traits<F>::template ArgT<1>>;

// StateProxy and StateStorage concepts guarantees that StateT is a valid state type
template <typename F>
using parse_handler_state_type = parse_handler_proxy<F>::StateT;

template <typename F>
    requires concepts::VariantState<parse_handler_state_type<F>> &&
                 meta::is_part_of_variant<std::remove_cvref_t<typename meta::function_traits<F>::template ArgT<2>>,
                                          parse_handler_state_type<F>>
using parse_variant_handler_state = std::remove_cvref_t<typename meta::function_traits<F>::template ArgT<2>>;

template <typename EventArgs>
struct EventContext;

template <typename... EventArgs>
struct EventContext<std::tuple<EventArgs...>> {
    template <typename F>
    struct validate_handler : std::false_type {};

    // If StateT is an enum, than the handler must take StateT&
    // Otherwise StateT is a variant and the handler must take one of its possible types
    template <typename F>
        requires concepts::HandlerWithoutState<F, parse_handler_proxy<F>, EventArgs...> ||
                 concepts::HandlerWithState<F,
                                            parse_handler_proxy<F>,
                                            std::conditional_t<concepts::EnumState<parse_handler_state_type<F>>,
                                                               parse_handler_state_type<F>,
                                                               parse_variant_handler_state<F>>,
                                            EventArgs...>
    struct validate_handler<F> : std::true_type {};

    template <typename F>
        requires validate_handler<F>::value
    using parse_handler_type =
        std::conditional_t<concepts::HandlerWithoutState<F, parse_handler_proxy<F>, EventArgs...>,
                           void, // Cannot determine if it is NoState or AnyState
                           HandlerStateTypes::State>;
};

} // namespace detail

template <concepts::Event Event_,
          typename F,
          concepts::HandlerStateType Type_ =
              detail::EventContext<typename Event_::HandlerArgs>::template parse_handler_type<F>>
    requires(detail::EventContext<typename Event_::HandlerArgs>::template validate_handler<F>::value)
struct HandlerMeta {
    using HandlerT = F;
    using Event = Event_;
    using Type = Type_;
    using StateT = detail::parse_handler_state_type<F>;
    using StateStorageT = detail::parse_handler_proxy<F>::StorageT;
    static_assert(
        !std::same_as<Type, void>,
        "You must explicitly specify type of a handler that doesn't accept a state parameter (NoState or AnyState)");

    F handler;
};

namespace concepts {

template <typename StateT, typename StateStorageT, typename... HandlerMetas>
concept check_handlers =
    ((std::same_as<typename HandlerMetas::StateT, StateT> &&
      std::same_as<typename HandlerMetas::StateStorageT, StateStorageT> &&
      ::tg_stater::detail::EventContext<typename HandlerMetas::Event::HandlerArgs>::template validate_handler<
          typename HandlerMetas::HandlerT>::value) &&
     ...);

template <typename HandlerMeta, typename State, typename StateT, typename StateStorageT>
concept BelongsToStateHandler =
    VariantState<StateT> && meta::is_part_of_variant<State, StateT> &&
    check_handlers<StateT, StateStorageT, HandlerMeta> &&
    std::same_as<State, detail::parse_variant_handler_state<typename HandlerMeta::HandlerT>>;

} // namespace concepts

template <typename F>
using MessageStateHandler = HandlerMeta<Events::Message, F, HandlerStateTypes::State>;

template <typename F>
constexpr auto MessageNoStateHandler(const F& f) {
    return HandlerMeta<Events::Message, F, HandlerStateTypes::NoState>{f};
}

} // namespace tg_stater

#endif // INCLUDE_bot_lib_handler_handler_meta
