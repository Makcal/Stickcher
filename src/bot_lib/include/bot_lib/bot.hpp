#ifndef INCLUDE_bot_lib_bot
#define INCLUDE_bot_lib_bot

#include "bot_lib/meta.hpp"
#include "bot_lib/state.hpp"
#include "bot_lib/state_storage/common.hpp"
#include "bot_lib/state_storage/memory.hpp"

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

template <concepts::State StateT, concepts::StateStorage<StateT> StateStorageT, typename Dependency>
class Stater {
    static constexpr bool isVariantState = concepts::VariantState<StateT>;

    StateStorageT stateStorage_;

    struct HandlerHelper {
        struct HanlderFilter {
            template <auto Meta, auto... Metas>
            static consteval decltype(auto) find(const auto& filter) {
                if constexpr (filter.template operator()<decltype(Meta)>())
                    return std::tuple_cat(std::tuple{Meta}, find<Metas...>(filter));
                else
                    return find<Metas...>(filter);
            }

            template <auto... Metas>
                requires(sizeof...(Metas) == 0)
            static consteval std::tuple<> find(const auto& /*filter*/) {
                return std::tuple{};
            }
        };

        static constexpr auto noStateFilter = []<typename Meta>() {
            return std::same_as<typename Meta::Type, HandlerStateTypes::NoState>;
        };
        static constexpr auto anyStateFilter = []<typename Meta>() {
            return std::same_as<typename Meta::Type, HandlerStateTypes::AnyState>;
        };
        static constexpr auto specificStateFilter = []<typename Meta>() {
            return std::same_as<typename Meta::Type, HandlerStateTypes::State>;
        };
        template <meta::is_part_of_variant<StateT> State>
            requires isVariantState
        static constexpr auto variantStateFilterByState =
            []<typename Meta>() { return concepts::BelongsToStateHandler<Meta, State, StateT, StateStorageT>; };

        static consteval decltype(auto) findHandlers(const auto& filter) {
            return HanlderFilter::template find<HandlerMetas...>(filter);
        }
    };

    template <auto... EventHandlers>
    void eventHandler(TgBot::Bot& bot, const TgBot::Message::Ptr& mp) {
        if (!mp || !mp->chat)
            throw std::runtime_error("Null message or chat.");

        const TgBot::Message& message = *mp;
        const auto chatId = message.chat->id;
        StateT* const mCurrentState = stateStorage_[chatId];
        const StateProxy stateProxy{stateStorage_, chatId};

        if (mCurrentState) {
            StateT& currentStateRef = *mCurrentState;
            if constexpr (!isVariantState) {
                // enum state handlers
                const StateT currentStateCopy = currentStateRef;
                std::apply(
                    [&](auto&&... handlers) {
                        ((handlers.state == currentStateCopy
                              ? handlers.handler(bot.getApi(), stateProxy, currentStateRef, message)
                              : void()),
                         ...);
                    },
                    HandlerHelper::findHandlers(HandlerHelper::enumStateFilter));
            } else {
                // variant state handlers
                std::visit(
                    [&](auto& state) {
                        std::apply(
                            [&](auto&&... handlers) {
                                (handlers.handler(bot.getApi(), stateProxy, state, message), ...);
                            },
                            HandlerHelper::findHandlers(HandlerHelper::template variantStateFilterByState<
                                                        std::remove_cvref_t<decltype(state)>>));
                    },
                    currentStateRef);
            }
        } else {
            // no state handlers
            std::apply([&](auto&&... handlers) { (handlers.handler(bot.getApi(), stateProxy, message), ...); },
                       HandlerHelper::findHandlers(HandlerHelper::noStateFilter));
        }
        // any state handlers
        std::apply([&](auto&&... handlers) { (handlers.handler(bot.getApi(), stateProxy, message), ...); },
                   HandlerHelper::findHandlers(HandlerHelper::anyStateFilter));

        std::println(std::clog, "{}: Get message from chat {}", std::chrono::system_clock::now(), chatId);
    }

  public:
    explicit constexpr Stater(const StateStorageT& stateStorage = StateStorageT{}) : stateStorage_{stateStorage} {}

    template <std::same_as<TgBot::Bot> Bot>
    void start(Bot&& bot) {
        bot.getEvents().onAnyMessage([&](const TgBot::Message::Ptr& mp) { anyMessageHandler(bot, mp); });

        std::println(std::clog, "Bot has started.");
        TgBot::TgLongPoll longPoll{std::forward<Bot>(bot)};
        while (true) {
            try {
                longPoll.start();
            } catch (const std::exception& e) {
                std::println(std::clog, "{}", e.what());
            }
        }
    }
};

template <typename StateT, typename StateStorageT = MemoryStateStorage<StateT>>
struct Setup {
    template <auto... HandlerMetas>
    using Stater = Stater<StateT, StateStorageT, HandlerMetas...>;
};

template <typename StateT, auto... HandlerMetas>
using DefaultStater = Setup<StateT>::template Stater<HandlerMetas...>;

} // namespace tg_stater
#endif // INCLUDE_bot_lib_bot
