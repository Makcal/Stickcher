#pragma once

#include <boost/locale/generator.hpp>
#include <rapidfuzz/fuzz.hpp>
#include <uuid.h>

#include <concepts>
#include <cstdlib>
#include <cstring>
#include <format>
#include <functional>
#include <locale>
#include <memory>
#include <optional>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace utils {

inline const std::locale utf8locale = boost::locale::generator{}("en_US.UTF-8"); // NOLINT(cert-err58-cpp)

inline const char* getenvWithError(const char* key) {
    const char* value = std::getenv(key);
    if (value == nullptr)
        throw std::runtime_error(std::format("Environmental variable {} is not set.", key));
    return value;
}

inline uuids::uuid generateUuid() {
    static std::mt19937 random{static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count())};
    static uuids::uuid_random_generator generator{random};
    return generator();
}

template <typename CharT, std::ranges::forward_range R, typename Proj = std::identity>
    requires std::same_as<CharT,
                          std::ranges::range_value_t<std::invoke_result_t<Proj, std::ranges::range_reference_t<R>>>> ||
             std::convertible_to<const CharT*, std::ranges::range_value_t<R>>
auto ratioToAll(R&& choices, // NOLINT(*-forward)
                std::basic_string_view<CharT> query,
                double scoreCutoff = 0.0,
                Proj proj = {}) -> std::vector<std::pair<std::ranges::borrowed_iterator_t<R>, double>> {
    using namespace std::ranges;
    std::vector<std::pair<borrowed_iterator_t<R>, double>> results;
    rapidfuzz::fuzz::CachedPartialRatio<CharT> scorer(query);

    for (auto it = begin(choices); it != end(choices); ++it) {
        double score = scorer.similarity(std::invoke(proj, *it), scoreCutoff);
        if (score >= scoreCutoff)
            results.emplace_back(it, score);
    }

    return results;
}

template <typename T>
std::shared_ptr<T> make_shared(T&& t) {
    return std::make_shared<std::remove_cv_t<T>>(std::forward<T>(t));
}

template <typename T>
T parse(const char* s) {
    T value;
    if (std::from_chars(s, s + std::strlen(s), value).ec == std::errc{})
        return value;
    throw std::runtime_error(std::format("Cannot parse {}", s));
}

template <typename T>
std::optional<T> parseSafe(const char* s) {
    T value;
    if (std::from_chars(s, s + std::strlen(s), value).ec == std::errc{})
        return value;
    return std::nullopt;
}

} // namespace utils
