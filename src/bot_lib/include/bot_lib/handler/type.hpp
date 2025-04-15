#ifndef INCLUDE_bot_lib_handler_type
#define INCLUDE_bot_lib_handler_type

#include "bot_lib/meta.hpp"

namespace tg_stater {

struct HandlerTypes {
    HandlerTypes() = delete;

    struct NoState {
        static constexpr bool takesState = false;
    };

    struct AnyState {
        static constexpr bool takesState = false;
    };

    struct State {
        static constexpr bool takesState = true;
    };
};

namespace concepts {

template <typename T>
concept HandlerType =
    meta::one_of<T, HandlerTypes::NoState, HandlerTypes::AnyState, HandlerTypes::State>;

} // namespace concepts

} // namespace tg_stater

#endif // INCLUDE_bot_lib_handler_type
