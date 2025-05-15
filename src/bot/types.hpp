#pragma once

#include "db/models.hpp"

#include <tg_stater/tg_types.hpp>
#include <tgbot/Api.h>
#include <tgbot/types/InlineKeyboardButton.h>
#include <tgbot/types/Message.h>

#include <vector>

using UserId = db::models::UserId;
using ChatId = tg_stater::ChatIdT;
using MessageId = decltype(TgBot::Message::messageId);

using StickerPackId = decltype(db::models::StickerPack::id);
using StickerPackName = decltype(db::models::StickerPack::name);
using StickerFileId = decltype(db::models::Sticker::fileId);
using StickerFileUniqueId = decltype(db::models::Sticker::fileUniqueId);

using BotRef = const TgBot::Api&;
using InlineKeyboard = std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>>;
