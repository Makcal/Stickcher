#include <type_traits>

#include <tgbot/tgbot.h>

namespace stater {

template <typename T>
concept scoped_enum = std::is_scoped_enum_v<T>;

template <scoped_enum State>
class Stater {
    TgBot::Bot bot;
};
} // namespace stater

