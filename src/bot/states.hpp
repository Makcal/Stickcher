#pragma once

#include "types.hpp"

#include <tg_stater/state_storage/common.hpp>
#include <tg_stater/state_storage/memory.hpp>

#include <variant>

namespace states {

using namespace db::models;

struct PackList {};

struct PackCreateEnterName {};

struct PackView {
    StickerPackId packId;
};

struct PackDeletion {};

struct StickerDeletion {};

struct StickerAddition {};

struct TagAddition {};

using State =
    std::variant<PackList, PackCreateEnterName, PackView, PackDeletion, StickerDeletion, StickerAddition, TagAddition>;

using StateManager = tg_stater::StateProxy<tg_stater::MemoryStateStorage<State>>;

} // namespace states
