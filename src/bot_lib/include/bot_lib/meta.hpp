#ifndef INCLUDE_bot_lib_meta
#define INCLUDE_bot_lib_meta

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <variant>

namespace tg_stater::meta {

namespace detail {

template <template <typename...> typename T>
struct IsOfTemplateImpl {
    template <typename C>
    struct check : std::false_type {};

    template <typename... Args>
    struct check<T<Args...>> : std::true_type {};
};

template <typename T, typename V, std::size_t I>
struct ElementInVariantCheck : std::bool_constant<std::is_same_v<T, std::variant_alternative_t<I, V>> ||
                                                  ElementInVariantCheck<T, V, I - 1>::value> {};

template <typename T, typename V>
struct ElementInVariantCheck<T, V, 0> : std::bool_constant<std::is_same_v<T, std::variant_alternative_t<0, V>>> {};

template <typename T, typename V>
concept IsPartOfVariantImpl = ElementInVariantCheck<T, V, std::variant_size_v<V> - 1>::value;

} // namespace detail

template <typename U, template <typename...> typename T>
concept is_of_template = detail::IsOfTemplateImpl<T>::template check<U>::value;

template <typename T, typename V>
concept is_part_of_variant = meta::is_of_template<V, std::variant> && detail::IsPartOfVariantImpl<T, V>;

template <template <typename...> typename T, typename... Args1>
struct curry {
    template <typename... Args2>
    using type = T<Args1..., Args2...>;
};

template <template <typename, typename...> typename T, typename Arg2>
struct flip {
    template <typename Arg1, typename... Args>
    using type = T<Arg1, Arg2, Args...>;
};

template <template <typename, typename...> typename T, typename Arg2>
struct flip_finite {
    template <typename Arg1, typename... Args>
    struct alias {
        using type = T<Arg1, Arg2, Args...>;
    };
    template <typename... Args>
    using type = typename alias<Args...>::type;
};

// Example:
/*
template <typename T1, typename T2>
struct TestPair {};
template <typename... Args>
using someTemplate = Curry<TestPair, double>::type<Args...>;
using someType = TestPair<double, double>;
static_assert(is_of_template<someTemplate>::check<someType>::value);
*/

template <typename T, typename... Ts>
concept one_of = (std::same_as<T, Ts> || ...);

} // namespace tg_stater::meta
#endif // INCLUDE_bot_lib_meta
