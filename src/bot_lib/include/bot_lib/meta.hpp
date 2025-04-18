#ifndef INCLUDE_bot_lib_meta
#define INCLUDE_bot_lib_meta

#include <brigand/algorithms/fold.hpp>
#include <brigand/algorithms/sort.hpp>

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>
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

namespace detail {

// function traits
template <typename ReturnT_, typename... Args>
struct general_function_traits {
    using ReturnT = ReturnT_;

    constexpr static std::size_t arg_count = sizeof...(Args);

    template <std::size_t I>
    using ArgT = std::tuple_element<I, std::tuple<Args...>>::type;
};

} // namespace detail

template <typename F>
struct function_traits;

template <typename F>
    requires requires { &F::operator(); }
struct function_traits<F> : function_traits<decltype(&F::operator())> {};

// for non-static non-mutable lambdas
template <typename T, typename ReturnT, typename... Args>
struct function_traits<ReturnT (T::*)(Args...) const> : detail::general_function_traits<ReturnT, Args...> {};

// for static lambdas and function pointers
template <typename ReturnT, typename... Args>
struct function_traits<ReturnT (*)(Args...)> : detail::general_function_traits<ReturnT, Args...> {};

// for function types
template <typename ReturnT, typename... Args>
struct function_traits<ReturnT(Args...)> : function_traits<ReturnT (*)(Args...)> {};

// Inspired by Haskell. See https://github.com/cpp-ru/ideas/issues/610 for description
template <typename...>
struct Proxy {};

// Inspired by Haskell DataKind operator (')
template <auto V>
using quote = std::integral_constant<decltype(V), V>;

// helper functions for sorting and aggregation
namespace detail {

template <typename T>
consteval auto callKey(auto key) {
    return key.template operator()<T>();
}

template <typename Key, typename L, typename R>
using TypeComparator = brigand::bool_<detail::callKey<L>(Key::value) < detail::callKey<R>(Key::value)>;

template <typename T, typename U>
struct append0 {
    using type = decltype(std::tuple_cat(std::declval<T>(), std::declval<std::tuple<U>>()));
};

template <typename T, typename U>
using append0_t = append0<T, U>::type;

template <typename... Ts>
struct last;

template <typename T, typename... Ts>
struct last<T, Ts...> {
    using type = last<Ts...>::type;
};

template <typename T>
struct last<T> {
    using type = T;
};

template <typename... Ts>
using last_t = last<Ts...>::type;

template <typename... Ts>
struct init;

template <typename... Ts>
using init_t = init<Ts...>::type;

template <typename T, typename... Ts>
struct init<T, Ts...> {
    using type = decltype(tuple_cat(std::declval<std::tuple<T>>(), std::declval<init_t<Ts...>>()));
};

template <typename T>
struct init<T> {
    using type = std::tuple<>;
};

template <typename T, typename U>
struct append1;

template <typename... Ts, typename U>
struct append1<std::tuple<Ts...>, U> {
    using type = append0_t<init_t<Ts...>, append0_t<last_t<Ts...>, U>>;
};

template <typename T, typename U>
using append1_t = append1<T, U>::type;

template <auto Key, typename State, typename Arg>
concept ShouldStartNewGroup =
    std::tuple_size_v<State> == 0 ||
    callKey<std::tuple_element_t<0, std::tuple_element_t<std::tuple_size_v<State> - 1, State>>>(Key) !=
        callKey<Arg>(Key);

template <typename Key, typename State, typename Arg>
using GroupByCompose = std::conditional_t<detail::ShouldStartNewGroup<Key::value, State, Arg>,
                                          detail::append0<State, std::tuple<Arg>>,
                                          detail::append1<State, Arg>>::type;

} // namespace detail

template <auto Key, typename... Args>
using sorted =
    brigand::sort<brigand::list<Args...>, brigand::bind<detail::TypeComparator, quote<Key>, brigand::_1, brigand::_2>>;

template <auto Key, typename... Args>
using group_by = brigand::fold<sorted<Key, Args...>,
                               std::tuple<>,
                               brigand::bind<detail::GroupByCompose, quote<Key>, brigand::_1, brigand::_2>>;

namespace detail {

template <typename T>
struct TupleToProxyImpl;

template <typename... Args>
struct TupleToProxyImpl<std::tuple<Args...>> {
    using type = Proxy<Args...>;
};

} // namespace detail

template <typename T>
using TupleToProxy = detail::TupleToProxyImpl<T>::type;

} // namespace tg_stater::meta
#endif // INCLUDE_bot_lib_meta
