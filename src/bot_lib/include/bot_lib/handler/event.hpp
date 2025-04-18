#ifndef INCLUDE_bot_lib_handler_event
#define INCLUDE_bot_lib_handler_event

#include "bot_lib/meta.hpp"
#include "bot_lib/state_storage/common.hpp"

#include <tgbot/types/CallbackQuery.h>
#include <tgbot/types/ChatJoinRequest.h>
#include <tgbot/types/ChatMemberUpdated.h>
#include <tgbot/types/ChosenInlineResult.h>
#include <tgbot/types/InlineQuery.h>
#include <tgbot/types/Message.h>
#include <tgbot/types/PollAnswer.h>
#include <tgbot/types/PreCheckoutQuery.h>
#include <tgbot/types/ShippingQuery.h>

namespace tg_stater {

struct EventCategories {
    EventCategories() = delete;

    struct Message {
        using CallbackArgs = std::tuple<const TgBot::Message&>;
        static StateKey getStateKey(const TgBot::Message::Ptr& mp) {
            if (!mp || !mp->chat) [[unlikely]]
                throw std::runtime_error("Null message or chat.");
            return {.chatId = mp->chat->id,
                    .threadId = mp->isTopicMessage ? std::optional{mp->messageThreadId} : std::nullopt};
        }
    };
    struct InlineQuery {
        using CallbackArgs = std::tuple<const TgBot::InlineQuery&>;
        static StateKey getStateKey(const TgBot::InlineQuery::Ptr& iqp) {
            if (!iqp || !iqp->from) [[unlikely]]
                throw std::runtime_error("Null inline query or user.");
            return {.chatId = iqp->from->id};
        }
    };
    struct ChosenInlineResult {
        using CallbackArgs = std::tuple<const TgBot::ChosenInlineResult&>;
        static StateKey getStateKey(const TgBot::ChosenInlineResult::Ptr& cirp) {
            if (!cirp || !cirp->from) [[unlikely]]
                throw std::runtime_error("Null inline query result or user.");
            return {.chatId = cirp->from->id};
        }
    };
    struct CallbackQuery {
        using CallbackArgs = std::tuple<const TgBot::CallbackQuery&>;
        static StateKey getStateKey(const TgBot::CallbackQuery::Ptr& cqp) {
            if (!cqp || !cqp->from) [[unlikely]]
                throw std::runtime_error("Null callback query or user.");
            return {.chatId = cqp->from->id};
        }
    };
    struct ShippingQuery {
        using CallbackArgs = std::tuple<const TgBot::ShippingQuery&>;
        static StateKey getStateKey(const TgBot::ShippingQuery::Ptr& sqp) {
            if (!sqp || !sqp->from) [[unlikely]]
                throw std::runtime_error("Null shipping query or user.");
            return {.chatId = sqp->from->id};
        }
    };
    struct PreCheckoutQuery {
        using CallbackArgs = std::tuple<const TgBot::PreCheckoutQuery&>;
        static StateKey getStateKey(const TgBot::PreCheckoutQuery::Ptr& pcqp) {
            if (!pcqp || !pcqp->from) [[unlikely]]
                throw std::runtime_error("Null precheckout query or user.");
            return {.chatId = pcqp->from->id};
        }
    };
    // Not supported.
    /* struct Poll {
     *     using CallbackArgs = std::tuple<const TgBot::Poll&>;
     * };
     */

    // Either for a chat or a user depeding on whether the poll is anonymous
    struct PollAnswer {
        using CallbackArgs = std::tuple<const TgBot::PollAnswer&>;
        static StateKey getStateKey(const TgBot::PollAnswer::Ptr& pap) {
            if (!pap || !(pap->voterChat || pap->user)) [[unlikely]]
                throw std::runtime_error("Null poll answer or chat/user.");
            return {.chatId = pap->voterChat ? pap->voterChat->id : pap->user->id};
        }
    };
    struct ChatMemberUpdated {
        using CallbackArgs = std::tuple<const TgBot::ChatMemberUpdated&>;
        static StateKey getStateKey(const TgBot::ChatMemberUpdated::Ptr& cmup) {
            if (!cmup || !cmup->chat) [[unlikely]]
                throw std::runtime_error("Null chat member update or chat.");
            return {.chatId = cmup->chat->id};
        }
    };
    struct ChatJoinRequest {
        using CallbackArgs = std::tuple<const TgBot::ChatJoinRequest&>;
        static StateKey getStateKey(const TgBot::ChatJoinRequest::Ptr& cjrp) {
            if (!cjrp || !cjrp->chat) [[unlikely]]
                throw std::runtime_error("Null chat join request or chat.");

            auto userId = cjrp->from->id;
            return {.chatId = userId};
        }
    };
};

struct Events {
    Events() = delete;

    // a non-command message
    struct Message {
        using Category = EventCategories::Message;
    };
    struct Command {
        using Category = EventCategories::Message;

        const char* command;
    };
    struct UnknownCommand {
        using Category = EventCategories::Message;
    };
    struct AnyMessage { // a regular message or a command
        using Category = EventCategories::Message;
    };
    struct EditedMessage {
        using Category = EventCategories::Message;
    };
    struct InlineQuery {
        using Category = EventCategories::InlineQuery;
    };
    struct ChosenInlineResult {
        using Category = EventCategories::ChosenInlineResult;
    };
    struct CallbackQuery {
        using Category = EventCategories::CallbackQuery;
    };
    struct ShippingQuery {
        using Category = EventCategories::ShippingQuery;
    };
    struct PreCheckoutQuery {
        using Category = EventCategories::PreCheckoutQuery;
    };
    // Not supported. You can hang a handler to the TgBot::Bot yourself befor passing it to the Stater.
    /* struct Poll {
     *     using Category = EventCategories::Poll;
     * };
     */

    struct PollAnswer {
        using Category = EventCategories::PollAnswer;
    };
    struct MyChatMember {
        using Category = EventCategories::ChatMemberUpdated;
    };
    struct ChatMember {
        using Category = EventCategories::ChatMemberUpdated;
    };
    struct ChatJoinRequest {
        using Category = EventCategories::ChatJoinRequest;
    };
};

namespace concepts {

template <typename T>
concept Event = meta::one_of<T,
                             Events::Message,
                             Events::Command,
                             Events::UnknownCommand,
                             Events::AnyMessage,
                             Events::EditedMessage,
                             Events::InlineQuery,
                             Events::ChosenInlineResult,
                             Events::CallbackQuery,
                             Events::ShippingQuery,
                             Events::PreCheckoutQuery,
                             // Events::Poll,
                             Events::PollAnswer,
                             Events::MyChatMember,
                             Events::ChatMember,
                             Events::ChatJoinRequest>;

} // namespace concepts

} // namespace tg_stater

#endif // INCLUDE_bot_lib_handler_event
