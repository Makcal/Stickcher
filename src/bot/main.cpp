#include "handlers.hpp"
#include "states.hpp"
#include "text_parser.hpp"
#include "utils.hpp"

#include <tg_stater/bot.hpp>
#include <tg_stater/dependencies.hpp>
#include <tgbot/Bot.h>

int main() {
    using namespace tg_stater;
    using namespace handlers;
    using namespace states;

    Setup<State, Dependencies<TextParser>>::Stater<noStateHandler,
                                                   startHandler,
                                                   packListButtonHandler,
                                                   packCreateHandler,
                                                   packCreateButtonHandler,
                                                   packViewButtonHandler,
                                                   packDeletionButtonHandler,
                                                   stickerAdditionButtonHandler,
                                                   stickerAdditionHandler,
                                                   tagAdditionButtonHandler,
                                                   tagAdditionHandler>
        bot{{}, {TextParser{utils::getenvWithError("TEXT_PARSER_URL")}}};

    bot.start(TgBot::Bot{utils::getenvWithError("BOT_TOKEN")});
}
