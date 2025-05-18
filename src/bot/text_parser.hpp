#pragma once

#include <httplib.h>

#include <string>

class TextParser {
  private:
    mutable httplib::Client http;

  public:
    explicit TextParser(const std::string& apiUrl) : http{apiUrl} {}

    std::string parse(const std::string& image) const {
        auto scanned = http.Post("/scan", image, "text/plain");
        return scanned->body;
    }
};
