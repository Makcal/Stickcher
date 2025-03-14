/*#include "db.hpp"*/
#include "bot_lib/bot.hpp"
#include "bot_lib/handler.hpp"
#include "bot_lib/state_storage/common.hpp"
#include "bot_lib/state_storage/memory.hpp"

/*#include <sqlite_orm/sqlite_orm.h>*/
#include <tgbot/Bot.h>
#include <tgbot/net/TgLongPoll.h>
#include <tgbot/types/Message.h>

#include <print>

enum struct State : char {
    A,
    B,
    C,
};

int main() {
    using namespace TgBot;
    using tg_stater::EnumStateHandler, tg_stater::NoStateHandler, tg_stater::AnyStateHandler;
    using StateStorage = tg_stater::StateProxy<State, tg_stater::MemoryStateStorage<State>>;

    Bot bot{"7686111024:AAH7FiefCD6YyP_PXp18vc93hMBEBzJa-3U"};
    /*std::unordered_map<decltype(Chat::id), State> states;*/
    /*bot.getEvents().onAnyMessage([&](const Message::Ptr& mp) {*/
    /*    auto chatId = mp->chat->id;*/
    /*    if (!states.contains(chatId)) {*/
    /*        bot.getApi().sendMessage(chatId, "default");*/
    /*        states[chatId] = State::A;*/
    /*        return;*/
    /*    }*/
    /**/
    /*    auto state = states[chatId];*/
    /*    switch (state) {*/
    /*    case State::A:*/
    /*        bot.getApi().sendMessage(chatId, "A");*/
    /*        states[chatId] = State::B;*/
    /*        break;*/
    /*    case State::B:*/
    /*        bot.getApi().sendMessage(chatId, "B");*/
    /*        states.erase(chatId);*/
    /*        break;*/
    /*    }*/
    /*});*/

    /*try {*/
    /*    std::println("Bot username: {}", bot.getApi().getMe()->username);*/
    /*    TgLongPoll lp{bot};*/
    /*    while (true) {*/
    /*        lp.start();*/
    /*    }*/
    /*} catch (...) {*/
    /*    std::println("Error");*/
    /*}*/

    auto default_ = [](const Message& m, Bot& bot, const StateStorage& ss) {
        bot.getApi().sendMessage(m.chat->id, "default. Now in A");
        ss.put(State::A);
    };
    auto always = [](const Message& m, Bot& bot, const StateStorage&) {
        bot.getApi().sendMessage(m.chat->id, "always");
    };
    auto stateA = [](State& state, const Message& m, Bot& bot, const StateStorage& /*ss*/) {
        bot.getApi().sendMessage(m.chat->id, "A");
        state = State::B;
    };
    auto stateB = [](State& state, const Message& m, Bot& bot, const StateStorage& /*ss*/) {
        bot.getApi().sendMessage(m.chat->id, "B");
        state = State::C;
    };
    auto stateC = [](State& state, const Message& m, Bot& bot, const StateStorage& /*ss*/) {
        bot.getApi().sendMessage(m.chat->id, "C");
        state = State::A;
    };

    tg_stater::DefaultStater<State,
                             NoStateHandler{default_},
                             AnyStateHandler{always},
                             EnumStateHandler{.state = State::A, .handler = stateA},
                             EnumStateHandler{.state = State::B, .handler = stateB},
                             EnumStateHandler{.state = State::C, .handler = stateC}>
        stater{std::move(bot)};
    std::println("Bot has started.");
    stater.start();
}
