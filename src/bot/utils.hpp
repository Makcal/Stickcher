#pragma once

#include <format>
#include <rapidfuzz/fuzz.hpp>

#include <charconv>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <optional>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <utility>

namespace utils {

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

template <std::ranges::forward_range R, typename Proj = std::identity>
std::vector<std::pair<std::ranges::borrowed_iterator_t<R>, double>>
ratioToAll(R&& choices, std::string_view query, double scoreCutoff = 0.0, Proj proj = {}) { // NOLINT(*-forward)
    using namespace std::ranges;
    std::vector<std::pair<borrowed_iterator_t<R>, double>> results;
    rapidfuzz::fuzz::CachedPartialRatio<char> scorer(query);

    for (auto it = begin(choices); it != end(choices); ++it) {
        double score = scorer.similarity(std::invoke(proj, *it), scoreCutoff);
        if (score >= scoreCutoff)
            results.emplace_back(it, score);
    }

    return results;
}

template <typename T>
std::shared_ptr<T> make_shared(T&& t) {
    return std::make_shared<T>(std::forward<T>(t));
}

template <typename T>
T parse(const char* s) {
    T value;
    if (std::from_chars(s, s + std::strlen(s), value).ec == std::errc{})
        return value;
    throw std::runtime_error(std::format("Cannot parse {}", s));
}

} // namespace utils
