#ifndef INCLUDE_bot_lib_state
#define INCLUDE_bot_lib_state

#include "bot_lib/meta.hpp"

#include <type_traits>
#include <variant>

namespace tg_stater {

struct AnyState {};
struct NoState {};

template <typename T>
concept UnionState = meta::is_of_template<std::variant, T>;

template <typename T>
concept EnumState = std::is_enum_v<T>;

template <typename T>
concept State = UnionState<T> || EnumState<T>;

} // namespace tg_stater
#endif // !INCLUDE_bot_lib_state
