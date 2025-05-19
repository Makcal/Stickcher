#include "handlers.hpp"
#include "settings.hpp"
#include "states.hpp"
#include "text_parser.hpp"
#include "utils.hpp"

#include <tg_stater/bot.hpp>
#include <tg_stater/dependencies.hpp>
#include <tgbot/Bot.h>

#include <cstddef>

int main() {
    using namespace tg_stater;
    using namespace handlers;
    using namespace states;

    const BotSettings settings{utils::parse<double>(utils::getenvWithError("SIMILARITY_THRESHOLD")),
                               utils::parse<std::size_t>(utils::getenvWithError("ASSOCIATION_LIMIT"))};
    Setup<State, Dependencies<TextParser, BotSettings>>::Stater<noStateHandler,
                                                                startHandler,
                                                                packListButtonHandler,
                                                                packCreateHandler,
                                                                packCreateButtonHandler,
                                                                packViewButtonHandler,
                                                                packDeletionButtonHandler,
                                                                stickerAdditionButtonHandler,
                                                                stickerAdditionHandler,
                                                                tagAdditionButtonHandler,
                                                                tagAdditionHandler,
                                                                stickerDeletionButtonHandler,
                                                                stickerDeletionHandler,
                                                                inlineSearchHandler>
        bot{{}, {TextParser{utils::getenvWithError("TEXT_PARSER_URL")}, settings}};

    bot.start(TgBot::Bot{utils::getenvWithError("BOT_TOKEN")});
}
