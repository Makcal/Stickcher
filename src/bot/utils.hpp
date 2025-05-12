#pragma once

#include <cstdlib>
#include <stdexcept>

namespace utils {

inline const char* getenvWithError(const char* key) {
    const char* value = std::getenv(key);
    if (value == nullptr)
        throw std::runtime_error(std::format("Environmental variable {} is not set.", key));
    return value;
}

} // namespace utils
