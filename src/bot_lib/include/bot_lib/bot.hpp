#ifndef INCLUDE_bot_lib_bot
#define INCLUDE_bot_lib_bot

#include "bot_lib/handler.hpp"
#include "bot_lib/state.hpp"
#include "bot_lib/state_storage/common.hpp"
#include "bot_lib/state_storage/memory.hpp"

#include <chrono>
#include <iostream>
#include <tgbot/Bot.h>
#include <tgbot/net/TgLongPoll.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/Chat.h>
#include <tgbot/types/Message.h>

#include <print>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

namespace tg_stater {

template <State StateT, StateStorage<StateT> StateStorageT, auto... HandlerMetas>
    requires(concepts::AnyHandler<decltype(HandlerMetas), StateT, StateStorageT> && ...)
class Stater {
    StateStorageT stateStorage_;
    TgBot::Bot bot_;

    template <auto Filter>
    struct StateFilter {
        template <auto Meta, auto... Metas>
        static consteval decltype(auto) find() {
            if constexpr (Filter.template operator()<decltype(Meta)>()) {
                return std::tuple_cat(std::tuple{Meta}, find<Metas...>());
            } else {
                return find<Metas...>();
            }
        }

        template <auto... Metas>
            requires(sizeof...(Metas) == 0)
        static consteval decltype(auto) find() {
            return std::tuple{};
        }
    };

    static constexpr auto noStateFilter = []<typename Meta>() {
        return concepts::NoStateHandler<Meta, StateT, StateStorageT>;
    };
    static constexpr auto anyStateFilter = []<typename Meta>() {
        return concepts::AnyStateHandler<Meta, StateT, StateStorageT>;
    };
    static constexpr auto enumStateFilter = []<typename Meta>() {
        return concepts::EnumStateHandler<Meta, StateT, StateStorageT>;
    };

    template <auto Filter>
    static consteval auto findStates() {
        return StateFilter<Filter>::template find<HandlerMetas...>();
    }

  public:
    template <typename BotT>
        requires std::is_same_v<std::remove_cvref_t<BotT>, TgBot::Bot>
    explicit Stater(BotT&& bot, const StateStorageT& stateStorage = StateStorageT{})
        : bot_{std::forward<BotT>(bot)}, stateStorage_{stateStorage} {
        bot_.getEvents().onAnyMessage([&](const TgBot::Message::Ptr& mp) {
            if (!mp || !mp->chat) {
                throw std::runtime_error("Null message or chat.");
            }
            const TgBot::Message& message = *mp;
            const auto chatId = message.chat->id;
            const auto mCurrentState = stateStorage_[chatId];
            const StateProxy stateProxy{stateStorage_, chatId};
            if (mCurrentState) {
                auto currentStateCopy = mCurrentState->get();
                auto& currentStateRef = *mCurrentState;
                std::apply(
                    [&](auto&&... args) {
                        ((args.state == currentStateCopy ? args.handler(currentStateRef, message, bot_, stateProxy)
                                                         : void()),
                         ...);
                    },
                    findStates<enumStateFilter>());
            } else {
                std::apply([&](auto&&... args) { (args.handler(message, bot_, stateProxy), ...); },
                           findStates<noStateFilter>());
            }
            std::apply([&](auto&&... args) { (args.handler(message, bot_, stateProxy), ...); },
                       findStates<anyStateFilter>());

            std::println(std::cerr, "{}: Get message from chat {}", std::chrono::system_clock::now(), chatId);
        });
    }

    void start() {
        TgBot::TgLongPoll longPoll{bot_};
        while (true) {
            try {
                longPoll.start();
            } catch (const std::exception& e) {
                std::println(std::cerr, "{}", e.what());
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
