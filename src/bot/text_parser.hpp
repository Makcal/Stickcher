#pragma once

#include <string>
#include <string_view>

class TextParser {
  private:
    std::string apiUrl;

  public:
    explicit TextParser(std::string_view apiUrl) : apiUrl(apiUrl) {}

    std::string parse(std::string_view image) const {
        return "stub";
    }
};
