#include "utils.hpp"

#include <tgbot/Bot.h>

int main() {
    TgBot::Bot bot(utils::getenvWithError("BOT_TOKEN"));
}
