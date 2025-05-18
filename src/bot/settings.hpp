#pragma once

#include <cstddef>

struct BotSettings {
    // The miminum similarity score needed for sticker to be retrieved
    double similarityThreshold;
    // The maximum number of text associations to use for sticker search
    std::size_t associationLimit;

    explicit BotSettings(double similarityThreshold, std::size_t associationLimit)
        : similarityThreshold{similarityThreshold}, associationLimit{associationLimit} {}
};
