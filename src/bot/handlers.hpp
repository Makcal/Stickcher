#pragma once

#include "db/pack.hpp"
#include "render.hpp"
#include "states.hpp"

#include <tg_stater/handler/event.hpp>
#include <tg_stater/handler/handler.hpp>
#include <tg_stater/handler/type.hpp>
#include <tgbot/Api.h>
#include <tgbot/types/CallbackQuery.h>
#include <tgbot/types/Message.h>
#include <uuid.h>

#include <string>

namespace handlers {
// NOLINTBEGIN(*-avoid-c-arrays)
// NOLINTBEGIN(*-decay)

using namespace TgBot;
using namespace tg_stater;
using NoState = HandlerTypes::NoState;
using AnyState = HandlerTypes::AnyState;

using namespace states;
using namespace render;

using MessageRef = const Message&;
using CallbackQueryRef = const CallbackQuery&;
using BotRef = const Api&;
using SMRef = const StateManager&;

namespace detail {

inline bool filterPublicMessage(MessageRef m, BotRef bot) {
    if (!m.from || m.chat->id != m.from->id) {
        bot.sendMessage(m.chat->id, "Setup stickers in private messages");
        return true;
    }
    return false;
}

} // namespace detail

constexpr auto handleNoState = [](MessageRef m, BotRef bot) {
    if (m.text.starts_with("/start"))
        return;
    if (detail::filterPublicMessage(m, bot))
        return;
    bot.sendMessage(m.chat->id, "Use /start please");
};
using noStateHandler = Handler<Events::AnyMessage{}, handleNoState, NoState{}>;

constexpr char startCmd[] = "start";
constexpr auto start = [](MessageRef m, BotRef bot, SMRef stateManager) {
    if (detail::filterPublicMessage(m, bot))
        return;
    stateManager.put(PackList{});
    renderPackListMessage(m.from->id, m.chat->id, bot);
};
using startHandler = Handler<Events::Command{startCmd}, start, AnyState{}>;

constexpr auto packListButtonCallback = [](PackList&, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    if (cq.data == "create") {
        stateManager.put(PackCreateEnterName{});
        bot.sendMessage(cq.message->chat->id, "Enter name:");
        return;
    }
    stateManager.put(PackView{});
    renderPackView(*uuids::uuid::from_string(cq.data), cq.message->chat->id, bot);
};
using packListButtonHandler = Handler<Events::CallbackQuery{}, packListButtonCallback>;

constexpr auto createPack = [](PackCreateEnterName&, MessageRef m, BotRef bot, SMRef stateManager) {
    StickerPackRepository::create(m.text, m.from->id);
    stateManager.put(PackList{});
    renderPackListMessage(m.from->id, m.chat->id, bot);
};
using packCreateHandler = Handler<Events::Message{}, createPack>;

// NOLINTEND(*-decay)
// NOLINTEND(*-avoid-c-arrays)
} // namespace handlers
