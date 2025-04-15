#ifndef INCLUDE_bot_lib_handler_event
#define INCLUDE_bot_lib_handler_event

#include "bot_lib/meta.hpp"

#include <tgbot/Bot.h>

namespace TgBot {

class Message;
class InlineQuery;
class ChosenInlineResult;
class CallbackQuery;
class ShippingQuery;
class PreCheckoutQuery;
class Poll;
class PollAnswer;
class ChatMemberUpdated;
class ChatJoinRequest;

} // namespace TgBot

namespace tg_stater {

struct EventCategories {
    EventCategories() = delete;

    struct Message {
        using CallbackArgs = std::tuple<const TgBot::Message&>;
    };
    struct InlineQuery {
        using CallbackArgs = std::tuple<const TgBot::InlineQuery&>;
    };
    struct ChosenInlineResult {
        using CallbackArgs = std::tuple<const TgBot::ChosenInlineResult&>;
    };
    struct CallbackQuery {
        using CallbackArgs = std::tuple<const TgBot::CallbackQuery&>;
    };
    struct ShippingQuery {
        using CallbackArgs = std::tuple<const TgBot::ShippingQuery&>;
    };
    struct PreCheckoutQuery {
        using CallbackArgs = std::tuple<const TgBot::PreCheckoutQuery&>;
    };
    struct Poll {
        using CallbackArgs = std::tuple<const TgBot::Poll&>;
    };
    struct PollAnswer {
        using CallbackArgs = std::tuple<const TgBot::PollAnswer&>;
    };
    struct ChatMemberUpdated {
        using CallbackArgs = std::tuple<const TgBot::ChatMemberUpdated&>;
    };
    struct ChatJoinRequest {
        using CallbackArgs = std::tuple<const TgBot::ChatJoinRequest&>;
    };
};

struct Events {
    Events() = delete;

    struct Message { // a non-command message
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
    struct Poll {
        using Category = EventCategories::Poll;
    };
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
                             Events::Poll,
                             Events::PollAnswer,
                             Events::MyChatMember,
                             Events::ChatMember,
                             Events::ChatJoinRequest>;

} // namespace concepts

} // namespace tg_stater

#endif // INCLUDE_bot_lib_handler_event
