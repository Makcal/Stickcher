#ifndef INCLUDE_bot_lib_bot
#define INCLUDE_bot_lib_bot

#include "bot_lib/dependencies.hpp"
#include "bot_lib/handler/callback.hpp"
#include "bot_lib/handler/event.hpp"
#include "bot_lib/handler/type.hpp"
#include "bot_lib/meta.hpp"
#include "bot_lib/state.hpp"
#include "bot_lib/state_storage/common.hpp"
#include "bot_lib/state_storage/memory.hpp"

#include <tgbot/Api.h>
#include <tgbot/Bot.h>
#include <tgbot/net/TgLongPoll.h>
#include <tgbot/types/Message.h>

#include <chrono>
#include <concepts>
#include <iostream>
#include <ostream>
#include <print>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace TgBot {
class Bot;
} // namespace TgBot

namespace tg_stater {

namespace detail {

template <concepts::State StateT, concepts::StateStorage<StateT> StateStorageT, typename Dependencies, typename Handler>
struct HandlerValidator {
  private:
    using FT = decltype(Handler::f);
    using FirstArgT = std::remove_cvref_t<typename meta::function_traits<FT>::template ArgT<0>>;
    static constexpr bool takesState = concepts::StateOption<FirstArgT, StateT>;

  public:
    using Callback_ = Callback<Handler::f,
                               StateT,
                               std::conditional_t<takesState, FirstArgT, NulloptStateOption>,
                               Handler::event,
                               Handler::type,
                               StateStorageT,
                               Dependencies>;
};

// CheckIsCallback
template <typename T>
struct CheckIsCallback : std::false_type {};

// CheckIsCallback
template <auto F,
          concepts::State StateT,
          concepts::OptionalStateOption<StateT> StateOptionT,
          concepts::Event auto Event,
          concepts::HandlerType auto HandlerType,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT>
struct CheckIsCallback<Callback<F, StateT, StateOptionT, Event, HandlerType, StateStorageT, DependenciesT>>
    : std::true_type {};

struct HandlerFinder {
  private:
    template <auto F, typename... Callbacks_>
    struct HandlerFilter;

    template <auto F, typename Callback, typename... Callbacks_>
    struct HandlerFilter<F, Callback, Callbacks_...> {
      private:
        static constexpr bool match = F.template operator()<Callback>();
        using other = HandlerFilter<F, Callbacks_...>::type;

      public:
        using type =
            std::conditional_t<match,
                               decltype(std::tuple_cat(std::declval<std::tuple<Callback>>(), std::declval<other>())),
                               other>;
    };

    template <auto F>
    struct HandlerFilter<F> {
        using type = std::tuple<>;
    };

  public:
    static constexpr auto noStateFilter = []<typename Callback>() {
        return std::same_as<typename Callback::TypeT, HandlerTypes::NoState>;
    };
    static constexpr auto anyStateFilter = []<typename Callback>() {
        return std::same_as<typename Callback::TypeT, HandlerTypes::AnyState>;
    };
    template <typename StateOptionT>
    static constexpr auto filterByStateOption = []<typename Callback>() {
        return std::same_as<typename Callback::TypeT, HandlerTypes::State> &&
               std::same_as<StateOptionT, typename Callback::StateOption>;
    };
    template <typename EventT>
    static constexpr auto filterByEventType =
        []<typename Callback>() { return std::same_as<EventT, typename Callback::EventT>; };

    template <auto Filter, typename... Callbacks_>
    using find = HandlerFilter<Filter, Callbacks_...>::type;
};

// Main implementation class.
template <concepts::State StateT,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT,
          typename... Callbacks>
    requires(detail::CheckIsCallback<Callbacks>::value && ...)
class StaterBase {
    StateStorageT stateStorage;
    DependenciesT dependencies;

    // Helper function to invoke handlers
    template <typename... Handlers, typename... Args>
    static constexpr void invokeHandlers(meta::Proxy<std::tuple<Handlers...>> /*unused*/, Args&&... args) {
        (Handlers::func(std::forward<Args>(args)...), ...);
    }

    template <typename... EventCallbacks, typename... EventArgs>
    void handleEvent(TgBot::Bot& bot, const StateKey& stateKey, EventArgs... eventArgs) {
        const TgBot::Api& api = bot.getApi();
        StateT* const mCurrentState = stateStorage[stateKey];
        const StateProxy stateProxy{stateStorage, stateKey};

        if (mCurrentState) {
            StateT& currentStateRef = *mCurrentState;
            std::visit(
                [&](auto& state) {
                    using StateOptionT = std::remove_reference_t<decltype(state)>;
                    constexpr auto stateFilter = HandlerFinder::filterByStateOption<StateOptionT>;
                    using StateHandlers = HandlerFinder::find<stateFilter, EventCallbacks...>;

                    invokeHandlers(meta::Proxy<StateHandlers>{}, state, eventArgs..., api, stateProxy, dependencies);
                },
                currentStateRef);
        } else {
            // no state handlers
            using NoStateHandlers = HandlerFinder::find<HandlerFinder::noStateFilter, EventCallbacks...>;
            invokeHandlers(meta::Proxy<NoStateHandlers>{}, eventArgs..., api, stateProxy, dependencies);
        }
        // any state handlers
        using AnyStateHandlers = HandlerFinder::find<HandlerFinder::anyStateFilter, EventCallbacks...>;
        invokeHandlers(meta::Proxy<AnyStateHandlers>{}, eventArgs..., api, stateProxy, dependencies);
    }

    static StateKey getKeyFromMessage(const TgBot::Message::Ptr& mp) {
        if (!mp || !mp->chat)
            throw std::runtime_error("Null message or chat.");

        const auto chatId = mp->chat->id;
        const auto threadId = mp->messageThreadId;
        return {.chatId = chatId};
    }

    template <typename... EventCallbacks, typename... Args>
    constexpr void invokeEventHandler(meta::Proxy<std::tuple<EventCallbacks...>> /*unused*/, Args&&... args) {
        handleEvent<EventCallbacks...>(std::forward<Args>(args)...);
    }

    void setup(TgBot::Bot& bot) {
        bot.getEvents().onNonCommandMessage([&](const TgBot::Message::Ptr& mp) {
            constexpr auto stateFilter = HandlerFinder::filterByEventType<Events::Message>;
            using EventCallbacks = HandlerFinder::find<stateFilter, Callbacks...>;
            const StateKey key = getKeyFromMessage(mp);

            invokeEventHandler(meta::Proxy<EventCallbacks>{}, bot, key, *mp);
            std::println(std::clog, "{}: Get message from chat {}", std::chrono::system_clock::now(), key);
        });
    }

  public:
    explicit constexpr StaterBase(const StateStorageT& stateStorage = StateStorageT{},
                                  const DependenciesT& dependencies = DependenciesT{})
        : stateStorage{stateStorage}, dependencies{dependencies} {}

    // rvalue reference means taking the ownership
    void start(TgBot::Bot&& bot) { // NOLINT(*-rvalue-reference-param-not-moved)
        setup(bot);

        std::println(std::clog, "Bot has started.");
        TgBot::TgLongPoll longPoll{bot};
        while (true) {
            try {
                longPoll.start();
            } catch (const std::exception& e) {
                std::println(std::clog, "{}", e.what());
            }
        }
    }
};

} // namespace detail

// Converts each `Handler` to a `Callback`
template <concepts::State StateT,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT,
          typename... Handlers>
using Stater =
    detail::StaterBase<StateT,
                       StateStorageT,
                       DependenciesT,
                       typename detail::HandlerValidator<StateT, StateStorageT, DependenciesT, Handlers>::Callback_...>;

template <typename StateT, typename Dependencies = Dependencies<>, typename StateStorageT = MemoryStateStorage<StateT>>
struct Setup {
    template <typename... Handlers>
    using Stater = Stater<StateT, StateStorageT, Dependencies, Handlers...>;
};

template <typename StateT, typename... Handlers>
using DefaultStater = Setup<StateT>::template Stater<Handlers...>;

} // namespace tg_stater
#endif // INCLUDE_bot_lib_bot
