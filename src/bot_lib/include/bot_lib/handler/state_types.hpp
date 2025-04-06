#ifndef INCLUDE_bot_lib_handler_state_types
#define INCLUDE_bot_lib_handler_state_types

#include "bot_lib/meta.hpp"

#include <tgbot/types/CallbackQuery.h>
#include <tgbot/types/ChatJoinRequest.h>
#include <tgbot/types/ChatMemberUpdated.h>
#include <tgbot/types/ChosenInlineResult.h>
#include <tgbot/types/InlineQuery.h>
#include <tgbot/types/Message.h>
#include <tgbot/types/Poll.h>
#include <tgbot/types/PollAnswer.h>
#include <tgbot/types/PreCheckoutQuery.h>
#include <tgbot/types/ShippingQuery.h>

namespace TgBot {
class Api;
} // namespace TgBot

namespace tg_stater {

struct HandlerStateTypes final {
    HandlerStateTypes() = delete;

    struct NoState {};

    struct AnyState {};

    struct State {};
};

namespace concepts {

template <typename T>
concept HandlerStateType =
    meta::one_of<T, HandlerStateTypes::NoState, HandlerStateTypes::AnyState, HandlerStateTypes::State>;

} // namespace concepts

} // namespace tg_stater

#endif // INCLUDE_bot_lib_handler_state_types
