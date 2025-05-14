#pragma once

#include <cstdlib>
#include <random>
#include <stdexcept>

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

} // namespace utils
