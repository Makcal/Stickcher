#include "handlers.hpp"
#include "states.hpp"
#include "utils.hpp"

#include <tg_stater/bot.hpp>
#include <tg_stater/handler/event.hpp>
#include <tg_stater/handler/handler.hpp>
#include <tgbot/Bot.h>

int main() {
    using namespace tg_stater;
    using NoState = HandlerTypes::NoState;
    using AnyState = HandlerTypes::AnyState;
    using namespace handlers;

    // NOLINTBEGIN(*-decay)
    Setup<State>::Stater<
        Handler<Events::AnyMessage{}, noState, NoState{}>,
        Handler<Events::Command{startCmd}, start, AnyState{}>
    > bot;
    // NOLINTEND(*-decay)

    bot.start(TgBot::Bot{utils::getenvWithError("BOT_TOKEN")});
}
