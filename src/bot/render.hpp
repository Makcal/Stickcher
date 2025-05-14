#pragma once

#include "db/pack.hpp"
#include "types.hpp"

#include <tgbot/Api.h>
#include <tgbot/types/CallbackQuery.h>
#include <tgbot/types/InlineKeyboardButton.h>
#include <tgbot/types/Message.h>
#include <uuid.h>

#include <format>
#include <memory>
#include <ranges>
#include <string_view>
#include <utility>

namespace render {

using namespace TgBot;
using namespace db;
using namespace db::models;

namespace detail {

template <typename T>
std::shared_ptr<T> make_shared(T&& t) {
    return std::make_shared<T>(std::forward<T>(t));
}

inline std::shared_ptr<InlineKeyboardButton> makeCallbackButton(std::string_view text, std::string_view data) {
    return make_shared(InlineKeyboardButton{.text = std::string(text), .callbackData = std::string(data)});
}

inline std::shared_ptr<InlineKeyboardMarkup> makeKeyboardMarkup(InlineKeyboard&& keyboard) {
    auto markup = std::make_shared<InlineKeyboardMarkup>();
    markup->inlineKeyboard = std::move(keyboard);
    return markup;
}

} // namespace detail

inline void renderPackList(UserId userId, ChatId chatId, BotRef bot) {
    auto packs = StickerPackRepository::getUserPacks(userId);

    InlineKeyboard keyboard(1 + ((packs.size() + 1) / 2)); // ceiling
    keyboard[0].push_back(detail::makeCallbackButton("Add new", "create"));
    for (auto [i, p] : std::views::enumerate(packs)) {
        if (i % 2 == 0)
            keyboard[1 + (i / 2)].reserve(2);
        keyboard[1 + (i / 2)].push_back(detail::makeCallbackButton(p.name, uuids::to_string(p.id)));
    }

    bot.sendMessage(chatId, "Your packs:", nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
}

inline void renderPackNamePrompt(ChatId chatId, BotRef bot) {
    InlineKeyboard keyboard(1);
    keyboard[0].push_back(detail::makeCallbackButton("Cancel", "cancel"));
    bot.sendMessage(chatId, "Enter name:", nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
}

inline void renderPackView(StickerPackId packId, ChatId chatId, BotRef bot) {
    auto packName = StickerPackRepository::getName(packId);

    InlineKeyboard keyboard(2);
    keyboard[0].reserve(2);
    keyboard[1].reserve(2);
    keyboard[0].push_back(detail::makeCallbackButton("Back", "back"));
    keyboard[0].push_back(detail::makeCallbackButton("Delete", "delete"));
    keyboard[1].push_back(detail::makeCallbackButton("Add sticker", "add_sticker"));
    keyboard[1].push_back(detail::makeCallbackButton("Delete sticker", "delete_sticker"));

    bot.sendMessage(
        chatId, std::format("Pack {}", packName), nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
}

inline void renderPackDeleteConfirmation(ChatId chatId, BotRef bot) {
    InlineKeyboard keyboard(1);
    keyboard[0].reserve(2);
    keyboard[0].push_back(detail::makeCallbackButton("Cancel", "cancel"));
    keyboard[0].push_back(detail::makeCallbackButton("Yes", "confirm"));
    bot.sendMessage(
        chatId, "Are you sure to delete this pack?", nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
}

} // namespace render
