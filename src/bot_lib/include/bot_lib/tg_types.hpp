#ifndef INCLUDE_bot_lib_chat_id_type
#define INCLUDE_bot_lib_chat_id_type

#include <tgbot/types/Chat.h>
#include <tgbot/types/Message.h>
#include <tgbot/types/User.h>

#include <type_traits>

namespace tg_stater {

using UserIdT = decltype(TgBot::User::id);
using ChatIdT = decltype(TgBot::Chat::id);
using ChatUserIdT = std::common_type_t<UserIdT, ChatIdT>;
using ThreadIdT = decltype(TgBot::Message::messageThreadId);

} // namespace tg_stater
#endif
