#ifndef INCLUDE_bot_lib_handler_events
#define INCLUDE_bot_lib_handler_events

#include "bot_lib/meta.hpp"

namespace tg_stater {

struct EventTypes final {
    EventTypes() = delete;

    struct Message {
        using HandlerArgs = std::tuple<const TgBot::Message&>;
    };
    struct InlineQuery {
        using HandlerArgs = std::tuple<const TgBot::InlineQuery::Ptr&>;
    };
    struct ChosenInlineResult {
        using HandlerArgs = std::tuple<const TgBot::ChosenInlineResult::Ptr&>;
    };
    struct CallbackQuery {
        using HandlerArgs = std::tuple<const TgBot::CallbackQuery::Ptr&>;
    };
    struct ShippingQuery {
        using HandlerArgs = std::tuple<const TgBot::ShippingQuery::Ptr&>;
    };
    struct PreCheckoutQuery {
        using HandlerArgs = std::tuple<const TgBot::PreCheckoutQuery::Ptr&>;
    };
    struct Poll {
        using HandlerArgs = std::tuple<const TgBot::Poll::Ptr&>;
    };
    struct PollAnswer {
        using HandlerArgs = std::tuple<const TgBot::PollAnswer::Ptr&>;
    };
    struct ChatMemberUpdated {
        using HandlerArgs = std::tuple<const TgBot::ChatMemberUpdated::Ptr&>;
    };
    struct ChatJoinRequest {
        using HandlerArgs = std::tuple<const TgBot::ChatJoinRequest::Ptr&>;
    };
};

struct Events final {
    Events() = delete;

    struct Message { // a non-command message
        using HandlerType = EventTypes::Message;
    };
    struct Command {
        using HandlerType = EventTypes::Message;

        std::string command;
    };
    struct UnknownCommand {
        using HandlerType = EventTypes::Message;
    };
    struct AnyMessage { // a regular message or a command
        using HandlerType = EventTypes::Message;
    };
    struct EditedMessage {
        using HandlerType = EventTypes::Message;
    };
    struct InlineQuery {
        using HandlerType = EventTypes::InlineQuery;
    };
    struct ChosenInlineResult {
        using HandlerType = EventTypes::ChosenInlineResult;
    };
    struct CallbackQuery {
        using HandlerType = EventTypes::CallbackQuery;
    };
    struct ShippingQuery {
        using HandlerType = EventTypes::ShippingQuery;
    };
    struct PreCheckoutQuery {
        using HandlerType = EventTypes::PreCheckoutQuery;
    };
    struct Poll {
        using HandlerType = EventTypes::Poll;
    };
    struct PollAnswer {
        using HandlerType = EventTypes::PollAnswer;
    };
    struct MyChatMember {
        using HandlerType = EventTypes::ChatMemberUpdated;
    };
    struct ChatMember {
        using HandlerType = EventTypes::ChatMemberUpdated;
    };
    struct ChatJoinRequest {
        using HandlerType = EventTypes::ChatJoinRequest;
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

#endif // INCLUDE_bot_lib_handler_events
