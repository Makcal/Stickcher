#pragma once

#include "db/models.hpp"

#include <tg_stater/tg_types.hpp>
#include <tgbot/types/InlineKeyboardButton.h>

#include <vector>

using UserId = db::models::UserId;
using ChatId = tg_stater::ChatIdT;

using StickerPackId = decltype(db::models::StickerPack::id);
using StickerPackName = decltype(db::models::StickerPack::name);

using InlineKeyboard = std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>>;
