#pragma once

#include "tables.hpp"

#include <variant>

struct PackList {};

struct PackCreateEnterName {};

struct PackView {
    decltype(tables::StickerPack::id) packId;
};

struct PackDeletion {};

struct StickerDeletion {};

struct StickerAddition {};

struct TagAddition {};

using State =
    std::variant<PackList, PackCreateEnterName, PackView, PackDeletion, StickerDeletion, StickerAddition, TagAddition>;
