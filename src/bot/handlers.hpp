#pragma once

#include "db/pack.hpp"
#include "states.hpp"
#include "types.hpp"

#include <memory>
#include <string>
#include <tgbot/Api.h>
#include <tgbot/types/Message.h>

#include <ranges>
#include <utility>
#include <uuid.h>

namespace handlers {
// NOLINTBEGIN(*-avoid-c-arrays)

using namespace TgBot;
using namespace tg_stater;
using namespace states;
using namespace db;

constexpr auto noState = [](const Message& m, const Api& bot) {
    if (m.text.starts_with("/start"))
        return;
    if (!m.from || m.chat->id != m.from->id) {
        bot.sendMessage(m.chat->id, "Setup stickers in private messages");
        return;
    }
    bot.sendMessage(m.chat->id, "Use /start please");
};

constexpr char startCmd[] = "start";
constexpr auto start = [](const Message& m, const Api& bot, const StateManager& stateManager) {
    if (!m.from || m.chat->id != m.from->id) {
        bot.sendMessage(m.chat->id, "Setup stickers in private messages");
        return;
    }

    stateManager.put(PackList{});
    auto packs = StickerPackRepository::getUserPacks(m.from->id);

    InlineKeyboard keyboard((packs.size() + 1) / 2); // ceiling
    for (auto [i, p] : std::views::enumerate(packs)) {
        if (i % 2 == 0)
            keyboard[i / 2].reserve(2);
        keyboard[i / 2].push_back(std::make_shared<InlineKeyboardButton>(
            InlineKeyboardButton{.text = std::move(p.name), .callbackData = uuids::to_string(p.id)}));
    }
    InlineKeyboardMarkup markup;
    markup.inlineKeyboard = std::move(keyboard);

    bot.sendMessage(
        m.chat->id, "Your packs:", nullptr, nullptr, std::make_shared<InlineKeyboardMarkup>(std::move(markup)));
};

// NOLINTEND(*-avoid-c-arrays)
} // namespace handlers
