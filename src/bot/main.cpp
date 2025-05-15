#include "handlers.hpp"
#include "states.hpp"
#include "utils.hpp"

#include <tg_stater/bot.hpp>
#include <tgbot/Bot.h>

int main() {
    using namespace tg_stater;
    using namespace handlers;
    using namespace states;

    Setup<State>::Stater<noStateHandler,
                         startHandler,
                         packListButtonHandler,
                         packCreateHandler,
                         packCreateButtonHandler,
                         packViewButtonHandler,
                         packDeletionButtonHandler>
        bot;

    bot.start(TgBot::Bot{utils::getenvWithError("BOT_TOKEN")});
}
