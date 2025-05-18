#pragma once

#include <httplib.h>

#include <format>
#include <stdexcept>
#include <string>

class TextParser {
  private:
    mutable httplib::Client http;

  public:
    explicit TextParser(const std::string& apiUrl) : http{apiUrl} {}

    std::string parse(const std::string& image) const {
        auto scanned = http.Post("/scan", image, "text/plain");
        if (scanned.error() != httplib::Error::Success || !scanned->body.c_str())
            throw std::runtime_error(std::format("Parsing request error {}", static_cast<int>(scanned.error())));
        if (scanned->status != httplib::StatusCode::OK_200)
            throw std::runtime_error(std::format("Parsing error: {}", scanned->body));
        return scanned->body;
    }
};
