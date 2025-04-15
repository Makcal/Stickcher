#ifndef INCLUDE_bot_lib_handler_callback
#define INCLUDE_bot_lib_handler_callback

#include "bot_lib/handler/event.hpp"
#include "bot_lib/handler/type.hpp"
#include "bot_lib/meta.hpp"
#include "bot_lib/state.hpp"
#include "bot_lib/state_storage/common.hpp"

#include <functional>
#include <tuple>
#include <type_traits>

namespace TgBot {
class Api;
} // namespace TgBot

namespace tg_stater {

namespace detail {

template <concepts::State StateT,
          concepts::OptionalStateOption<StateT> StateOptionT,
          concepts::HandlerType auto HandlerType,
          typename StateStorageProxy,
          typename EventArgs,
          typename Dependencies>
struct CallbackHelper;

template <concepts::State StateT,
          concepts::OptionalStateOption<StateT> StateOptionT,
          concepts::HandlerType auto HandlerType,
          typename StateStorageProxyT,
          typename... EventArgs,
          typename Dependencies>
struct CallbackHelper<StateT, StateOptionT, HandlerType, StateStorageProxyT, std::tuple<EventArgs...>, Dependencies> {
    static constexpr bool takesState = decltype(HandlerType)::takesState;
    static_assert(!(takesState && concepts::IsNulloptStateOption<StateOptionT>),
                  "If the handler must take a state, the state option must be specified.");

    // using FuncT = std::conditional_t<
    //     takesState,
    //     std::function<void(
    //         StateOptionT&, const EventArgs&..., const TgBot::Api&, const StateStorageProxyT&, const Dependencies&)>,
    //     std::function<void(const EventArgs&..., const TgBot::Api&, const StateStorageProxyT&, const Dependencies&)>>;

    template <typename F, bool takeState, typename... Args>
    static constexpr bool invocableWithExtra =
        takeState ? std::is_invocable_v<F, StateOptionT&, const EventArgs&..., const Args&...>
                  : std::is_invocable_v<F, const EventArgs&..., const Args&...>;
};

} // namespace detail

/*
 * Valid signatures:
 * from void(const EventArgs&...)
 * up to void([StateOption&, ]const EventArgs&..., const TgBot::Api&, const StateStorageProxy&, const Dependencies&)
 *
 * Everything except for EventArgs is optional. "Stateless" handlers can't take state.
 * For the lists of EventArgs see CallbackArgs member at handler/event.hpp
 */
template <auto F,
          concepts::State StateT,
          concepts::OptionalStateOption<StateT> StateOptionT,
          concepts::Event auto Event,
          concepts::HandlerType auto HandlerType,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT>
class Callback {
  public:
    using EventT = decltype(Event);
    using TypeT = decltype(HandlerType);

  private:
    using FT = decltype(F);
    using SProxyT = StateProxy<StateStorageT>;
    using Helper = detail::CallbackHelper<StateT,
                                          StateOptionT,
                                          HandlerType,
                                          SProxyT,
                                          typename EventT::Category::CallbackArgs,
                                          DependenciesT>;
    // using Func = Helper::type;
    using ApiRef = const TgBot::Api&;
    using SProxyRef = const SProxyT&;
    using DepRef = const DependenciesT&;

    static constexpr bool takesState = Helper::takesState;

    static constexpr auto transform() { // NOLINT(*-cognitive-complexity)
        // Inspired by the approach of "userver".
        // https://github.com/userver-framework/userver/blob/develop/libraries/easy/include/userver/easy.hpp
        // NOLINTBEGIN(*-magic-numbers)
        static constexpr unsigned matches =
            (Helper::template invocableWithExtra<FT, false, ApiRef, SProxyRef, DepRef> << 0) |
            (Helper::template invocableWithExtra<FT, false, SProxyRef, DepRef> << 1) |
            (Helper::template invocableWithExtra<FT, false, ApiRef, DepRef> << 2) |
            (Helper::template invocableWithExtra<FT, false, DepRef> << 3) |
            (Helper::template invocableWithExtra<FT, false, ApiRef, SProxyRef> << 4) |
            (Helper::template invocableWithExtra<FT, false, SProxyRef> << 5) |
            (Helper::template invocableWithExtra<FT, false, ApiRef> << 6) |
            (Helper::template invocableWithExtra<FT, false> << 7) |
            (-static_cast<unsigned>(takesState) &
             ((Helper::template invocableWithExtra<FT, true, ApiRef, SProxyRef, DepRef> << 8) |
              (Helper::template invocableWithExtra<FT, true, SProxyRef, DepRef> << 9) |
              (Helper::template invocableWithExtra<FT, true, ApiRef, DepRef> << 10) |
              (Helper::template invocableWithExtra<FT, true, DepRef> << 11) |
              (Helper::template invocableWithExtra<FT, true, ApiRef, SProxyRef> << 12) |
              (Helper::template invocableWithExtra<FT, true, SProxyRef> << 13) |
              (Helper::template invocableWithExtra<FT, true, ApiRef> << 14) |
              (Helper::template invocableWithExtra<FT, true> << 15)));
        static_assert(matches != 0, "Invalid signature. See class comment for available signatures.");
        static_assert((matches & (matches - 1)) == 0,
                      "Several valid signatures are applicable. This is not supported.");

        return []<typename... EventArgs>(meta::Proxy<std::tuple<EventArgs...>>) { // NOLINT(*-cognitive-complexity)
            if constexpr (!takesState) {
                return [](const EventArgs&... args, ApiRef api, SProxyRef proxy, DepRef deps) {
                    if constexpr (matches >> 0U == 1) {
                        F(args..., api, proxy, deps);
                    } else if constexpr (matches >> 1U == 1) {
                        F(args..., proxy, deps);
                    } else if constexpr (matches >> 2U == 1) {
                        F(args..., api, deps);
                    } else if constexpr (matches >> 3U == 1) {
                        F(args..., deps);
                    } else if constexpr (matches >> 4U == 1) {
                        F(args..., api, proxy);
                    } else if constexpr (matches >> 5U == 1) {
                        F(args..., proxy);
                    } else if constexpr (matches >> 6U == 1) {
                        F(args..., api);
                    } else if constexpr (matches >> 7U == 1) {
                        F(args...);
                    };
                };
            } else {
                return [](StateOptionT& state, const EventArgs&... args, ApiRef api, SProxyRef proxy, DepRef deps) {
                    if constexpr (matches >> 0U == 1) {
                        F(args..., api, proxy, deps);
                    } else if constexpr (matches >> 1U == 1) {
                        F(args..., proxy, deps);
                    } else if constexpr (matches >> 2U == 1) {
                        F(args..., api, deps);
                    } else if constexpr (matches >> 3U == 1) {
                        F(args..., deps);
                    } else if constexpr (matches >> 4U == 1) {
                        F(args..., api, proxy);
                    } else if constexpr (matches >> 5U == 1) {
                        F(args..., proxy);
                    } else if constexpr (matches >> 6U == 1) {
                        F(args..., api);
                    } else if constexpr (matches >> 7U == 1) {
                        F(args...);
                    } else if constexpr (matches >> 8U == 1) {
                        F(state, args..., api, proxy, deps);
                    } else if constexpr (matches >> 9U == 1) {
                        F(state, args..., proxy, deps);
                    } else if constexpr (matches >> 10U == 1) {
                        F(state, args..., api, deps);
                    } else if constexpr (matches >> 11U == 1) {
                        F(state, args..., deps);
                    } else if constexpr (matches >> 12U == 1) {
                        F(state, args..., api, proxy);
                    } else if constexpr (matches >> 13U == 1) {
                        F(state, args..., proxy);
                    } else if constexpr (matches >> 14U == 1) {
                        F(state, args..., api);
                    } else if constexpr (matches >> 15U == 1) {
                        F(state, args...);
                    };
                };
            }
        }(meta::Proxy<typename EventT::Category::CallbackArgs>{});
        // NOLINTEND(*-magic-numbers)
    }

  public:
    Callback() = delete;

    static constexpr auto func = transform();
    static constexpr auto event = Event;
    static constexpr auto type = HandlerType;
    using StateOption = StateOptionT;
};

} // namespace tg_stater

#endif // INCLUDE_bot_lib_handler_callback
