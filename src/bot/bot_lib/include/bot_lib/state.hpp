#ifndef INCLUDE_bot_lib_state
#define INCLUDE_bot_lib_state

#include "bot_lib/meta.hpp"

#include <concepts>
#include <variant>

namespace tg_stater {

namespace detail {

struct NulloptStateOption {};

} // namespace detail

namespace concepts {

// template <typename T>
// concept EnumState = std::is_enum_v<T>;

// algebraic union
// template <typename T>
// concept VariantState = meta::is_of_template<T, std::variant>;

// template <typename T>
// concept State = VariantState<T> || EnumState<T>;

// algebraic union
template <typename T>
concept State = meta::is_of_template<T, std::variant>;

template <typename T, typename StateT>
concept StateOption = State<StateT> && meta::is_part_of_variant<T, StateT>;

template <typename T>
concept IsNulloptStateOption = std::same_as<T, detail::NulloptStateOption>;

template <typename T, typename StateT>
concept OptionalStateOption = StateOption<T, StateT> || IsNulloptStateOption<T>;

} // namespace concepts

} // namespace tg_stater

#endif // !INCLUDE_bot_lib_state
