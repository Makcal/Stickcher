#ifndef INCLUDE_bot_lib_meta
#define INCLUDE_bot_lib_meta

#include <type_traits>

namespace tg_stater::meta {

namespace detail {

template <template <typename...> typename T>
struct is_of_template_impl {
    template <typename C>
    struct check : std::false_type {};

    template <typename... Args>
    struct check<T<Args...>> : std::true_type {};
};

} // namespace detail

template <template <typename...> typename T, typename U>
concept is_of_template = detail::is_of_template_impl<T>::template check<U>::value;

template <template <typename...> typename T, typename... Args1>
struct Curry {
    template <typename... Args2>
    using type = T<Args1..., Args2...>;
};

template <template <typename, typename...> typename T, typename Arg2>
struct Flip {
    template <typename Arg1, typename... Args>
    using type = T<Arg1, Arg2, Args...>;
};

template <template <typename, typename...> typename T, typename Arg2>
struct FlipFinite {
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

} // namespace tg_stater
#endif // INCLUDE_bot_lib_meta
