#pragma once

#include <tg_stater/tg_types.hpp>

#include <tgbot/types/InlineKeyboardButton.h>
#include <vector>

using UserId = tg_stater::UserIdT;
using ChatId = tg_stater::ChatIdT;

using InlineKeyboard = std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>>;
