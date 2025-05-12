#pragma once

#include "db/models.hpp"

#include <tg_stater/state_storage/common.hpp>
#include <tg_stater/state_storage/memory.hpp>

#include <variant>

namespace states {

using namespace db::models;

struct PackList {};

struct PackCreateEnterName {};

struct PackView {
    decltype(StickerPack::id) packId;
};

struct PackDeletion {};

struct StickerDeletion {};

struct StickerAddition {};

struct TagAddition {};

using State =
    std::variant<PackList, PackCreateEnterName, PackView, PackDeletion, StickerDeletion, StickerAddition, TagAddition>;

using StateManager = tg_stater::StateProxy<tg_stater::MemoryStateStorage<State>>;

} // namespace states
