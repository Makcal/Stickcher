#include "handlers.hpp"
#include "settings.hpp"
#include "states.hpp"
#include "text_parser.hpp"
#include "utils.hpp"

#include <tg_stater/bot.hpp>
#include <tg_stater/dependencies.hpp>
#include <tgbot/Bot.h>
#include <tgbot/net/CurlHttpClient.h>

#include <cstddef>
#include <cstdlib>

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
                                                                packImportHandler,
                                                                packImportButtonHandler,
                                                                packViewButtonHandler,
                                                                packDeletionButtonHandler,
                                                                stickerAdditionButtonHandler,
                                                                stickerAdditionHandler,
                                                                tagAdditionButtonHandler,
                                                                tagAdditionHandler,
                                                                stickerDeletionButtonHandler,
                                                                stickerDeletionHandler,
                                                                editorListHandler,
                                                                editorListButtonHandler,
                                                                inlineSearchHandler>
        bot{{}, {TextParser{utils::getenvWithError("TEXT_PARSER_URL")}, settings}};

    TgBot::CurlHttpClient http_client{};
    const char* proxy = std::getenv("BOT_PROXY");
    if (proxy != nullptr)
        http_client.setProxy(proxy);

    bot.start(TgBot::Bot{utils::getenvWithError("BOT_TOKEN"), http_client});
}
