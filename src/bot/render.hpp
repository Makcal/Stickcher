#pragma once

#include "db/pack.hpp"
#include "db/pack_sharing.hpp"
#include "states.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <tgbot/Api.h>
#include <tgbot/types/CallbackQuery.h>
#include <tgbot/types/InlineKeyboardButton.h>
#include <tgbot/types/Message.h>
#include <uuid.h>

#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace render {

using namespace TgBot;
using namespace db;
using namespace db::models;

namespace detail {

inline std::shared_ptr<InlineKeyboardButton> makeCallbackButton(std::string_view text, std::string_view data) {
    return utils::make_shared(InlineKeyboardButton{.text = std::string(text), .callbackData = std::string(data)});
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
    keyboard[0].reserve(2);
    keyboard[0].push_back(detail::makeCallbackButton("Add new", "create"));
    keyboard[0].push_back(detail::makeCallbackButton("Import", "import"));
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
    bot.sendMessage(chatId, "Enter a name", nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
}

inline void renderPackIdPrompt(ChatId chatId, BotRef bot) {
    InlineKeyboard keyboard(1);
    keyboard[0].push_back(detail::makeCallbackButton("Cancel", "cancel"));
    bot.sendMessage(chatId, "Enter a pack's id", nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
}

inline void renderPackView(StickerPackId packId, UserId userId, ChatId chatId, BotRef bot) {
    auto pack = StickerPackRepository::get(packId);
    bool isOwner = pack.ownerId == userId;
    bool isEditor = false;
    if (!isOwner)
        isEditor = PackSharingRepository::checkEditorRights(packId, userId);

    InlineKeyboard keyboard(isOwner || isEditor ? 2 : 1);
    keyboard[0].reserve(2);
    keyboard[0].push_back(detail::makeCallbackButton("Back", "back"));
    if (isOwner)
        keyboard[0].push_back(detail::makeCallbackButton("Delete", "delete"));
    else
        keyboard[0].push_back(detail::makeCallbackButton("Remove", "remove"));
    if (isOwner || isEditor) {
        keyboard[1].reserve(2);
        keyboard[1].push_back(detail::makeCallbackButton("Add sticker", "add_sticker"));
        keyboard[1].push_back(detail::makeCallbackButton("Delete sticker", "delete_sticker"));
    }

    bot.sendMessage(chatId,
                    std::format("Pack \"{}\"\n"
                                "Id: `{}`{}",
                                pack.name,
                                uuids::to_string(packId),
                                isOwner || isEditor
                                    ? "\nYou can add new stickers or new tags for exising stickers via \"Add sticker\""
                                    : ""),
                    nullptr,
                    nullptr,
                    detail::makeKeyboardMarkup(std::move(keyboard)),
                    "MarkdownV2");
}

inline void renderPackDeleteConfirmation(ChatId chatId, BotRef bot) {
    InlineKeyboard keyboard(1);
    keyboard[0].reserve(2);
    keyboard[0].push_back(detail::makeCallbackButton("Cancel", "cancel"));
    keyboard[0].push_back(detail::makeCallbackButton("Yes", "confirm"));
    bot.sendMessage(
        chatId, "Are you sure to delete this pack?", nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
}

inline void renderStickerPrompt(ChatId chatId, BotRef bot) {
    InlineKeyboard keyboard(1);
    keyboard[0].push_back(detail::makeCallbackButton("Cancel", "cancel"));
    bot.sendMessage(chatId, "Send me a sticker", nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
}

inline void renderTagPrompt(const states::TagAddition& state,
                            ChatId chatId,
                            BotRef bot,
                            std::optional<::MessageId> toEdit = std::nullopt) {
    InlineKeyboard keyboard(1);
    keyboard[0].reserve(2);
    keyboard[0].push_back(detail::makeCallbackButton("Cancel", "cancel"));
    if (state.tags.size() > 0)
        keyboard[0].push_back(detail::makeCallbackButton("Done", "done"));
    if (state.hasParsedTag) {
        keyboard.emplace(keyboard.begin());
        keyboard[0].push_back(detail::makeCallbackButton("Delete recogized", "delete_recognized"));
    }

    using namespace std::views;
    using std::ranges::to;
    auto text = std::format("You can add tags to the sticker.\n"
                            "When you finish, send me a next sticker or press \"Done\".\n"
                            "Tags:\n{}",
                            state.tags | join_with('\n') | to<std::string>());
    if (toEdit) {
        bot.editMessageText(text, chatId, *toEdit, "", "", nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
    } else {
        bot.sendMessage(chatId, text, nullptr, nullptr, detail::makeKeyboardMarkup(std::move(keyboard)));
    }
}

} // namespace render
