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
#include <exception>
#include <iostream>
#include <ostream>
#include <print>
#include <string_view>
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
    using CallbackT = Callback<Handler::f,
                               StateT,
                               std::conditional_t<takesState, FirstArgT, NulloptStateOption>,
                               Handler::event,
                               Handler::type,
                               StateStorageT,
                               Dependencies>;
};

// CheckIsCallback
template <typename T, typename StateT, typename StateStorageT, typename DependenciesT>
struct CheckIsCallback : std::false_type {};

// CheckIsCallback
template <auto F,
          concepts::State StateT,
          concepts::OptionalStateOption<StateT> StateOptionT,
          concepts::Event auto Event,
          concepts::HandlerType auto HandlerType,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT>
struct CheckIsCallback<Callback<F, StateT, StateOptionT, Event, HandlerType, StateStorageT, DependenciesT>,
                       StateT,
                       StateStorageT,
                       DependenciesT> : std::true_type {};

struct CallbackFinder {
  private:
    template <auto F, typename... Callbacks>
    struct CallbackFilter;

    template <auto F, typename Callback, typename... Callbacks>
    struct CallbackFilter<F, Callback, Callbacks...> {
      private:
        static constexpr bool match = F.template operator()<Callback>();
        using other = CallbackFilter<F, Callbacks...>::type;

      public:
        using type =
            std::conditional_t<match,
                               decltype(std::tuple_cat(std::declval<std::tuple<Callback>>(), std::declval<other>())),
                               other>;
    };

    template <auto F>
    struct CallbackFilter<F> {
        using type = std::tuple<>;
    };

  public:
    CallbackFinder() = delete;

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

    template <auto Filter, typename... Callbacks>
    using find = CallbackFilter<Filter, Callbacks...>::type;
};

// Main implementation class.
template <concepts::State StateT,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT,
          typename... Callbacks>
    requires(detail::CheckIsCallback<Callbacks, StateT, StateStorageT, DependenciesT>::value && ...)
class StaterBase {
    StateStorageT stateStorage;
    DependenciesT dependencies;

    // Helper function to invoke handlers
    template <typename... Callbacks_, typename... Args>
    static constexpr void invokeCallbacks(meta::Proxy<Callbacks_...> /*unused*/, Args&&... args) {
        (Callbacks_::func(std::forward<Args>(args)...), ...);
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
                    constexpr auto stateFilter = CallbackFinder::filterByStateOption<StateOptionT>;
                    using StateCallbacks = CallbackFinder::find<stateFilter, EventCallbacks...>;
                    invokeCallbacks(
                        meta::TupleToProxy<StateCallbacks>{}, state, eventArgs..., api, stateProxy, dependencies);
                },
                currentStateRef);
        } else {
            // no state handlers
            using NoStateCallbacks = CallbackFinder::find<CallbackFinder::noStateFilter, EventCallbacks...>;
            invokeCallbacks(meta::TupleToProxy<NoStateCallbacks>{}, eventArgs..., api, stateProxy, dependencies);
        }
        // any state handlers
        using AnyStateCallbacks = CallbackFinder::find<CallbackFinder::anyStateFilter, EventCallbacks...>;
        invokeCallbacks(meta::TupleToProxy<AnyStateCallbacks>{}, eventArgs..., api, stateProxy, dependencies);
    }

    template <typename... EventCallbacks, typename... Args>
    void handleEventProxy(meta::Proxy<EventCallbacks...> /*unused*/, Args&&... args) {
        handleEvent<EventCallbacks...>(std::forward<Args>(args)...);
    }

    static void logEvent(std::string_view event_type, const StateKey key) {
        std::println(
            std::clog, "{}: Trying to handle {} from chat {}", std::chrono::system_clock::now(), event_type, key);
    }

    template <typename Callbacks_>
    auto makeMessageHandler(std::string_view event, TgBot::Bot& bot) {
        return [&, event](const TgBot::Message::Ptr& mp) {
            const StateKey key = EventCategories::Message::getStateKey(mp);
            handleEventProxy(meta::TupleToProxy<Callbacks_>{}, bot, key, *mp);
            logEvent(event, key);
        };
    }

    template <typename EventT>
    using FindEventCallbacks = CallbackFinder::find<CallbackFinder::filterByEventType<EventT>, Callbacks...>;

    template <typename T>
    struct GroupCommandCallbacks;

    template <typename... CommandCallbacks>
    struct GroupCommandCallbacks<std::tuple<CommandCallbacks...>> {
        using type =
            meta::group_by<[]<typename C>() { return std::string_view{C::event.command}; }, CommandCallbacks...>;
    };

    void setup(TgBot::Bot& bot) {
        bot.getEvents().onNonCommandMessage(
            makeMessageHandler<FindEventCallbacks<Events::Message>>("non-command message", bot));
        using Grouped = GroupCommandCallbacks<FindEventCallbacks<Events::Command>>::type;
        [&]<typename... Groups>(meta::Proxy<Groups...>) {
            (bot.getEvents().onCommand({std::tuple_element_t<0, Groups>::event.command},
                                       makeMessageHandler<Groups>("command", bot)),
             ...);
        }(meta::TupleToProxy<Grouped>{});
        bot.getEvents().onUnknownCommand(
            makeMessageHandler<FindEventCallbacks<Events::UnknownCommand>>("unknown command", bot));
        bot.getEvents().onAnyMessage(makeMessageHandler<FindEventCallbacks<Events::AnyMessage>>("message", bot));
        bot.getEvents().onEditedMessage(
            makeMessageHandler<FindEventCallbacks<Events::EditedMessage>>("message edit", bot));
        bot.getEvents().onInlineQuery([&](const TgBot::InlineQuery::Ptr& iqp) {
            const StateKey key = EventCategories::InlineQuery::getStateKey(iqp);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::InlineQuery>>{}, bot, key, *iqp);
            logEvent("inline query", key);
        });
        bot.getEvents().onChosenInlineResult([&](const TgBot::ChosenInlineResult::Ptr& cirp) {
            const StateKey key = EventCategories::ChosenInlineResult::getStateKey(cirp);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::ChosenInlineResult>>{}, bot, key, *cirp);
            logEvent("chosen inline result", key);
        });
        bot.getEvents().onCallbackQuery([&](const TgBot::CallbackQuery::Ptr& cqp) {
            const StateKey key = EventCategories::CallbackQuery::getStateKey(cqp);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::CallbackQuery>>{}, bot, key, *cqp);
            logEvent("callback query", key);
        });
        bot.getEvents().onShippingQuery([&](const TgBot::ShippingQuery::Ptr& sqp) {
            const StateKey key = EventCategories::ShippingQuery::getStateKey(sqp);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::ShippingQuery>>{}, bot, key, *sqp);
            logEvent("shipping query", key);
        });
        bot.getEvents().onPreCheckoutQuery([&](const TgBot::PreCheckoutQuery::Ptr& pcqp) {
            const StateKey key = EventCategories::PreCheckoutQuery::getStateKey(pcqp);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::PreCheckoutQuery>>{}, bot, key, *pcqp);
            logEvent("precheckout query", key);
        });
        bot.getEvents().onPollAnswer([&](const TgBot::PollAnswer::Ptr& pap) {
            const StateKey key = EventCategories::PollAnswer::getStateKey(pap);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::PollAnswer>>{}, bot, key, *pap);
            logEvent("poll answer", key);
        });
        bot.getEvents().onMyChatMember([&](const TgBot::ChatMemberUpdated::Ptr& cmup) {
            const StateKey key = EventCategories::ChatMemberUpdated::getStateKey(cmup);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::MyChatMember>>{}, bot, key, *cmup);
            logEvent("chat member update for bot", key);
        });
        bot.getEvents().onChatMember([&](const TgBot::ChatMemberUpdated::Ptr& cmup) {
            const StateKey key = EventCategories::ChatMemberUpdated::getStateKey(cmup);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::ChatMember>>{}, bot, key, *cmup);
            logEvent("chat member update", key);
        });
        bot.getEvents().onChatJoinRequest([&](const TgBot::ChatJoinRequest::Ptr& cjrp) {
            const StateKey key = EventCategories::ChatJoinRequest::getStateKey(cjrp);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::ChatJoinRequest>>{}, bot, key, *cjrp);
            logEvent("chat join request", key);
        });
    }

  public:
    explicit constexpr StaterBase(StateStorageT stateStorage = StateStorageT{},
                                  DependenciesT dependencies = DependenciesT{})
        : stateStorage{std::move(stateStorage)}, dependencies{std::move(dependencies)} {}

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
                       typename detail::HandlerValidator<StateT, StateStorageT, DependenciesT, Handlers>::CallbackT...>;

template <typename StateT, typename Dependencies = Dependencies<>, typename StateStorageT = MemoryStateStorage<StateT>>
struct Setup {
    template <typename... Handlers>
    using Stater = Stater<StateT, StateStorageT, Dependencies, Handlers...>;
};

template <typename StateT, typename... Handlers>
using DefaultStater = Setup<StateT>::template Stater<Handlers...>;

} // namespace tg_stater
#endif // INCLUDE_bot_lib_bot
