#pragma once

#include "types.hpp"

#include <uuid.h>

#include <string>

namespace db::models {

struct Sticker {
    std::string fileUniqueId;
    std::string fileId;
};

struct StickerPack {
    uuids::uuid id;
    std::string name;
    UserId ownerId;
};

struct Association {
    std::string text;
    std::string stickerId; // fileUniqueId
    uuids::uuid packId;
};

} // namespace db::models
