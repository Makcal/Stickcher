#pragma once

#include "db/db.hpp"
#include "db/models.hpp"
#include "tables.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <sqlpp11/all_of.h>
#include <sqlpp11/insert.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/select.h>
#include <uuid.h>

#include <format>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace db {

using namespace models;
using namespace sqlpp;

class StickerPackRepository {
  public:
    struct StickerPackWithoutUser {
        StickerPackId id;
        StickerPackName name;
    };

    static std::vector<StickerPackWithoutUser> getUserPacks(UserId ownerId) {
        auto db = getDb();
        tables::StickerPack sp;
        auto packs = db(select(sp.id, sp.name).from(sp).where(sp.ownerId == ownerId));

        std::vector<StickerPackWithoutUser> result;
        result.reserve(packs.size());
        for (const auto& row : packs) {
            result.emplace_back(*uuids::uuid::from_string(row.id.value()), row.name);
        }
        return result;
    }

    struct StickerPackWithoutId {
        StickerPackName name;
        UserId ownerId;
    };

    static StickerPackWithoutId get(const StickerPackId& packId) {
        tables::StickerPack sp;
        if (const auto& row =
                getDb()(select(sqlpp::all_of(sp)).from(sp).where(sp.id == uuids::to_string(packId))).front())
            return StickerPackWithoutId{.name = row.name, .ownerId = row.ownerId};
        throw std::runtime_error(std::format("StickerPack {} not found", uuids::to_string(packId)));
    }

    static StickerPackName getName(const StickerPackId& packId) {
        tables::StickerPack sp;
        if (const auto& row = getDb()(select(sp.name).from(sp).where(sp.id == uuids::to_string(packId))).front())
            return row.name;
        throw std::runtime_error(std::format("StickerPack {} not found", uuids::to_string(packId)));
    }

    static StickerPackId create(std::string_view name, UserId ownerId) {
        StickerPackId packId = utils::generateUuid();
        tables::StickerPack sp;
        getDb()(insert_into(sp).set(sp.id = uuids::to_string(packId), sp.name = name, sp.ownerId = ownerId));
        return packId;
    }

    static void deletePack(const StickerPackId& packId) {
        tables::StickerPack sp;
        if (getDb()(remove_from(sp).where(sp.id == uuids::to_string(packId))) == 0)
            throw std::runtime_error(std::format("StickerPack {} not found", uuids::to_string(packId)));
    }
};

} // namespace db
