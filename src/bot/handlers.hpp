#pragma once

#include "db/pack.hpp"
#include "render.hpp"
#include "states.hpp"
#include "types.hpp"

#include <tg_stater/handler/event.hpp>
#include <tg_stater/handler/handler.hpp>
#include <tg_stater/handler/type.hpp>
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
    renderPackList(m.from->id, m.chat->id, bot);
};
using startHandler = Handler<Events::Command{startCmd}, start, AnyState{}>;

constexpr auto packListButtonCallback = [](PackList&, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    if (cq.data == "create") {
        stateManager.put(PackCreateEnterName{});
        renderPackNamePrompt(cq.message->chat->id, bot);
        return;
    }
    auto packId = *uuids::uuid::from_string(cq.data);
    stateManager.put(PackView{packId});
    renderPackView(packId, cq.message->chat->id, bot);
};
using packListButtonHandler = Handler<Events::CallbackQuery{}, packListButtonCallback>;

constexpr auto createPack = [](PackCreateEnterName&, MessageRef m, BotRef bot, SMRef stateManager) {
    StickerPackRepository::create(m.text, m.from->id);
    stateManager.put(PackList{});
    renderPackList(m.from->id, m.chat->id, bot);
};
using packCreateHandler = Handler<Events::Message{}, createPack>;

constexpr auto cancelPackCreation = [](PackCreateEnterName&, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    stateManager.put(PackList{});
    renderPackList(cq.from->id, cq.message->chat->id, bot);
};
using packCreateButtonHandler = Handler<Events::CallbackQuery{}, cancelPackCreation>;

constexpr auto packViewButtonCallback = [](PackView& state, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    auto userId = cq.from->id;
    auto chatId = cq.message->chat->id;
    if (cq.data == "back") {
        stateManager.put(PackList{});
        renderPackList(userId, chatId, bot);
        return;
    }
    if (cq.data == "delete") {
        stateManager.put(PackDeletion{state.packId});
        renderPackDeleteConfirmation(chatId, bot);
        return;
    }
    if (cq.data == "add_sticker") {
        return;
    }
    if (cq.data == "delete_sticker") {
        return;
    }
};
using packViewButtonHandler = Handler<Events::CallbackQuery{}, packViewButtonCallback>;

constexpr auto packDeletionButtonCallback =
    [](PackDeletion& state, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
        auto packId = state.packId;
        bot.answerCallbackQuery(cq.id);
        if (cq.data == "cancel") {
            stateManager.put(PackView{packId});
            renderPackView(packId, cq.message->chat->id, bot);
            return;
        }
        StickerPackRepository::deletePack(packId);
        stateManager.put(PackList{});
        renderPackList(cq.from->id, cq.message->chat->id, bot);
    };
using packDeletionButtonHandler = Handler<Events::CallbackQuery{}, packDeletionButtonCallback>;

// NOLINTEND(*-decay)
// NOLINTEND(*-avoid-c-arrays)
} // namespace handlers
