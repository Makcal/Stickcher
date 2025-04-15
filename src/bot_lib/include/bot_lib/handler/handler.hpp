#ifndef INCLUDE_bot_lib_handler_handler
#define INCLUDE_bot_lib_handler_handler

#include "bot_lib/handler/event.hpp"
#include "bot_lib/handler/type.hpp"

namespace tg_stater {

template <concepts::Event auto Event, auto F, concepts::HandlerType auto Type = HandlerTypes::State{}>
struct Handler {
    Handler() = delete;

    static constexpr auto event = Event;
    static constexpr auto f = F;
    static constexpr auto type = Type;
};

} // namespace tg_stater

#endif // INCLUDE_bot_lib_handler_handler
