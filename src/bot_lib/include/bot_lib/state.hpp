#ifndef INCLUDE_bot_lib_state
#define INCLUDE_bot_lib_state

#include "bot_lib/meta.hpp"

#include <type_traits>
#include <variant>

namespace tg_stater::concepts {

template <typename T>
concept EnumState = std::is_enum_v<T>;

// algebraic union
template <typename T>
concept VariantState = meta::is_of_template<T, std::variant>;

template <typename T>
concept State = VariantState<T> || EnumState<T>;

} // namespace tg_stater::concepts

#endif // !INCLUDE_bot_lib_state
