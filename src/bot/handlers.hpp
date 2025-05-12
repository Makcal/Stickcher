#pragma once

#include "states.hpp"

#include <tgbot/Api.h>
#include <tgbot/types/Message.h>

namespace handlers {
// NOLINTBEGIN(*-avoid-c-arrays)

using namespace TgBot;
using namespace tg_stater;
using namespace states;

constexpr auto noState = [](const Message& m, const Api& bot) {
    bot.sendMessage(m.chat->id, "Use /start please");
};

constexpr char startCmd[] = "start";
constexpr auto start = [](const Message& m, const Api& bot, const StateManager& stateManager) {
    stateManager.put(PackList{});
    bot.sendMessage(m.chat->id, "Your packs:");
};

// NOLINTEND(*-avoid-c-arrays)
} // namespace handlers
