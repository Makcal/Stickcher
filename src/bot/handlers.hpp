#pragma once

#include "db/pack.hpp"
#include "db/pack_sharing.hpp"
#include "db/sticker.hpp"
#include "render.hpp"
#include "settings.hpp"
#include "states.hpp"
#include "text_parser.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <tg_stater/handler/event.hpp>
#include <tg_stater/handler/handler.hpp>
#include <tg_stater/handler/type.hpp>
#include <tgbot/types/CallbackQuery.h>
#include <tgbot/types/InlineQuery.h>
#include <tgbot/types/Message.h>
#include <tgbot/types/Sticker.h>
#include <uuid.h>

#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace handlers {
// NOLINTBEGIN(*-avoid-c-arrays)
// NOLINTBEGIN(*-decay)
// NOLINTBEGIN(*-named-parameter)

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

inline void addSticker(const StickerPackId& packId,
                       ChatId chatId,
                       const TgBot::Sticker& sticker,
                       BotRef bot,
                       SMRef stateManager,
                       const TextParser& parser) {
    auto tags = StickerRepository::getTags(packId, sticker.fileUniqueId);
    if (!tags.empty()) { // re-add
        stateManager.put(TagAddition{packId, sticker.fileId, sticker.fileUniqueId, std::move(tags)});
        renderTagPrompt(std::get<TagAddition>(*stateManager.get()), chatId, bot);
        return;
    }

    bool isRegular = sticker.type == TgBot::Sticker::Type::Regular && !sticker.isAnimated && !sticker.isVideo;
    if (!isRegular && !sticker.thumbnail) {
        bot.sendMessage(chatId, "Error processing this sticker");
        renderStickerPrompt(chatId, bot);
        return;
    }

    auto imageId = isRegular ? sticker.fileId : sticker.thumbnail->fileId;
    auto imagePtr = bot.getFile(imageId);
    if (!imagePtr) {
        bot.sendMessage(chatId, "Error processing this sticker");
        renderStickerPrompt(chatId, bot);
        return;
    }
    std::string image = bot.downloadFile(imagePtr->filePath);

    std::string parsedText = parser.parse(image);
    if (parsedText.empty())
        stateManager.put(TagAddition{packId, sticker.fileId, sticker.fileUniqueId});
    else
        stateManager.put(TagAddition{packId, sticker.fileId, sticker.fileUniqueId, parsedText});
    renderTagPrompt(std::get<TagAddition>(*stateManager.get()), chatId, bot);
}

} // namespace detail

inline void handleNoState(MessageRef m, BotRef bot) {
    if (m.text.starts_with("/start"))
        return;
    if (detail::filterPublicMessage(m, bot))
        return;
    bot.sendMessage(m.chat->id, "Use /start please");
};
using noStateHandler = Handler<Events::AnyMessage{}, handleNoState, NoState{}>;

constexpr char startCmd[] = "start";
inline void start(MessageRef m, BotRef bot, SMRef stateManager) {
    if (detail::filterPublicMessage(m, bot))
        return;
    stateManager.put(PackList{});
    renderPackList(m.from->id, m.chat->id, bot);
};
using startHandler = Handler<Events::Command{startCmd}, start, AnyState{}>;

inline void packListButtonCallback(PackList&, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    auto chatId = cq.message->chat->id;
    if (cq.data == "create") {
        stateManager.put(PackCreateEnterName{});
        renderPackNamePrompt(chatId, bot);
        return;
    }
    if (cq.data == "import") {
        stateManager.put(PackImportEnterName{});
        renderPackIdPrompt(chatId, bot);
        return;
    }
    auto packId = *uuids::uuid::from_string(cq.data);
    stateManager.put(PackView{packId});
    renderPackView(packId, cq.from->id, chatId, bot);
};
using packListButtonHandler = Handler<Events::CallbackQuery{}, packListButtonCallback>;

inline void createPack(PackCreateEnterName&, MessageRef m, BotRef bot, SMRef stateManager) {
    StickerPackRepository::create(m.text, m.from->id);
    stateManager.put(PackList{});
    renderPackList(m.from->id, m.chat->id, bot);
};
using packCreateHandler = Handler<Events::Message{}, createPack>;

inline void cancelPackCreation(PackCreateEnterName&, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    if (cq.data == "cancel") {
        stateManager.put(PackList{});
        renderPackList(cq.from->id, cq.message->chat->id, bot);
    }
};
using packCreateButtonHandler = Handler<Events::CallbackQuery{}, cancelPackCreation>;

inline void importPack(PackImportEnterName&, MessageRef m, BotRef bot, SMRef stateManager) {
    auto maybePackId = uuids::uuid::from_string(m.text);
    auto chatId = m.chat->id;
    if (!maybePackId) {
        bot.sendMessage(chatId, "Invalid UUID format");
        renderPackIdPrompt(chatId, bot);
        return;
    }
    auto importResult = PackSharingRepository::import(*maybePackId, m.from->id);
    using ImportResult = PackSharingRepository::ImportResult;

    switch (importResult) {
    case ImportResult::Success:
        bot.sendMessage(chatId, "The pack was imported successfully");
        break;
    case ImportResult::NotExist:
        bot.sendMessage(chatId, "This pack does not exists");
        break;
    case ImportResult::ImportByOwner:
        bot.sendMessage(chatId, "You cannot import your own pack");
        break;
    case ImportResult::AlreadyImported:
        bot.sendMessage(chatId, "You've already imported this");
        break;
    }
    stateManager.put(PackList{});
    renderPackList(m.from->id, chatId, bot);
};
using packImportHandler = Handler<Events::Message{}, importPack>;

inline void cancelPackImport(PackImportEnterName&, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    if (cq.data == "cancel") {
        stateManager.put(PackList{});
        renderPackList(cq.from->id, cq.message->chat->id, bot);
    }
};
using packImportButtonHandler = Handler<Events::CallbackQuery{}, cancelPackImport>;

inline void packViewButtonCallback(PackView& state, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
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
    if (cq.data == "remove") {
        PackSharingRepository::remove(userId, state.packId);
        stateManager.put(PackList{});
        renderPackList(userId, chatId, bot);
        return;
    }
    if (cq.data == "add_sticker") {
        stateManager.put(StickerAddition{state.packId});
        renderStickerPrompt(chatId, bot);
        return;
    }
    if (cq.data == "delete_sticker") {
        stateManager.put(StickerDeletion{state.packId});
        renderStickerPrompt(chatId, bot);
        return;
    }
    if (cq.data == "editors") {
        renderEditorList(state.packId, chatId, bot);
        return;
    }
};
using packViewButtonHandler = Handler<Events::CallbackQuery{}, packViewButtonCallback>;

inline void packDeletionButtonCallback(PackDeletion& state, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    if (cq.data == "cancel") {
        renderPackView(state.packId, cq.from->id, cq.message->chat->id, bot);
        stateManager.put(PackView{state.packId});
        return;
    }
    if (cq.data == "confirm") {
        StickerPackRepository::deletePack(state.packId);
        stateManager.put(PackList{});
        renderPackList(cq.from->id, cq.message->chat->id, bot);
    }
};
using packDeletionButtonHandler = Handler<Events::CallbackQuery{}, packDeletionButtonCallback>;

inline void cancelStickerAddition(StickerAddition& state, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    if (cq.data == "cancel") {
        renderPackView(state.packId, cq.from->id, cq.message->chat->id, bot);
        stateManager.put(PackView{state.packId});
    }
};
using stickerAdditionButtonHandler = Handler<Events::CallbackQuery{}, cancelStickerAddition>;

inline void addSticker(StickerAddition& state, MessageRef m, BotRef bot, SMRef stateManager, const TextParser& parser) {
    auto chatId = m.chat->id;
    if (!m.sticker) {
        renderStickerPrompt(chatId, bot);
        return;
    }
    detail::addSticker(state.packId, chatId, *m.sticker, bot, stateManager, parser);
};
using stickerAdditionHandler = Handler<Events::Message{}, addSticker>;

inline void tagAdditionButtonCallback(TagAddition& state, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    auto chatId = cq.message->chat->id;
    if (cq.data == "cancel") {
        renderPackView(state.packId, cq.from->id, chatId, bot);
        stateManager.put(PackView{state.packId});
        return;
    }
    if (cq.data == "done") {
        if (state.tags.empty()) {
            bot.sendMessage(chatId, "You can't add a sticker without a tag");
            renderTagPrompt(state, chatId, bot);
        } else {
            StickerRepository::create(state);
            bot.sendMessage(chatId, "Sticker added");
            renderPackView(state.packId, cq.from->id, chatId, bot);
            stateManager.put(PackView{state.packId});
        }
        return;
    }
    if (cq.data == "delete_recognized") {
        state.hasParsedTag = false;
        state.tags.erase(state.tags.begin());
        renderTagPrompt(state, chatId, bot, cq.message->messageId);
        return;
    }
};
using tagAdditionButtonHandler = Handler<Events::CallbackQuery{}, tagAdditionButtonCallback>;

inline void
processTagMessage(TagAddition& state, MessageRef m, BotRef bot, SMRef stateManager, const TextParser& parser) {
    auto chatId = m.chat->id;
    if (!m.sticker) {
        if (!m.text.empty())
            state.tags.push_back(m.text);
        renderTagPrompt(state, chatId, bot);
        return;
    }
    if (state.tags.empty()) {
        bot.sendMessage(chatId, "You can't add a sticker without a tag. Back to the previous sticker");
        renderTagPrompt(state, chatId, bot);
        return;
    }
    StickerRepository::create(state);
    bot.sendMessage(chatId, "The previous sticker added");
    detail::addSticker(state.packId, chatId, *m.sticker, bot, stateManager, parser);
};
using tagAdditionHandler = Handler<Events::Message{}, processTagMessage>;

inline void searchStickers(const InlineQuery& iq, BotRef bot, const BotSettings& settings) {
    if (iq.query.empty()) {
        bot.answerInlineQuery(iq.id, {});
        return;
    }
    auto fileIds =
        StickerRepository::findAllByTag(iq.from->id, iq.query, settings.similarityThreshold, settings.associationLimit);
    std::vector<InlineQueryResult::Ptr> results;
    results.reserve(fileIds.size());
    std::unordered_set<StickerFileId> idSet;
    for (auto [i, id] : std::views::enumerate(fileIds)) {
        if (idSet.contains(id))
            continue;
        idSet.insert(id);
        auto p = std::make_shared<InlineQueryResultCachedSticker>();
        p->id = std::to_string(i);
        p->stickerFileId = std::move(id);
        results.emplace_back(p);
    }
    bot.answerInlineQuery(iq.id, results);
};
using inlineSearchHandler = Handler<Events::InlineQuery{}, searchStickers, AnyState{}>;

inline void cancelStickerDeletion(StickerDeletion& state, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    if (cq.data == "cancel") {
        renderPackView(state.packId, cq.from->id, cq.message->chat->id, bot);
        stateManager.put(PackView{state.packId});
    }
};
using stickerDeletionButtonHandler = Handler<Events::CallbackQuery{}, cancelStickerDeletion>;

inline void deleteSticker(StickerDeletion& state, MessageRef m, BotRef bot) {
    auto chatId = m.chat->id;
    if (!m.sticker) {
        renderStickerPrompt(chatId, bot);
        return;
    }
    if (StickerRepository::deleteFromPack(state.packId, m.sticker->fileUniqueId)) {
        bot.sendMessage(m.chat->id, "Sticker was removed from the pack");
    } else {
        bot.sendMessage(m.chat->id, "Sticker isn't in the pack");
    }
    renderStickerPrompt(chatId, bot);
};
using stickerDeletionHandler = Handler<Events::Message{}, deleteSticker>;

inline void changeEditors(EditorList& state, MessageRef m, BotRef bot) {
    auto mEditorId = utils::parseSafe<UserId>(m.text.data());
    if (mEditorId)
        PackSharingRepository::flipIsEditor(state.packId, *mEditorId);
    renderEditorList(state.packId, m.chat->id, bot);
}
using editorListHandler = Handler<Events::Message{}, changeEditors>;

inline void cancelEditorChange(EditorList& state, CallbackQueryRef cq, BotRef bot, SMRef stateManager) {
    bot.answerCallbackQuery(cq.id);
    if (cq.data == "back") {
        renderPackView(state.packId, cq.from->id, cq.message->chat->id, bot);
        stateManager.put(PackView{state.packId});
    }
};
using editorListButtonHandler = Handler<Events::CallbackQuery{}, cancelEditorChange>;

// NOLINTEND(*-named-parameter)
// NOLINTEND(*-decay)
// NOLINTEND(*-avoid-c-arrays)
} // namespace handlers
