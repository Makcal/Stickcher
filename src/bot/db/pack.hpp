#pragma once

#include "db/db.hpp"
#include "db/models.hpp"
#include "tables.hpp"
#include "types.hpp"
#include "utils.hpp"

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

class StickerPackRepository {
  public:
    struct StickerPackWithoutUser {
        StickerPackId id;
        StickerPackName name;
    };

    static std::vector<StickerPackWithoutUser> getUserPacks(UserId userId) {
        auto db = getDb();
        std::vector<StickerPackWithoutUser> result;

        tables::StickerPack sp;
        auto packs = db(select(sp.id, sp.name).from(sp).where(sp.ownerId == userId));

        tables::PackSharing ps;
        auto importedPacks =
            db(select(sp.id, sp.name).from(ps.join(sp).on(ps.packId == sp.id)).where(ps.userId == userId));

        result.reserve(packs.size() + importedPacks.size());
        for (const auto& row : packs)
            result.emplace_back(*uuids::uuid::from_string(row.id.value()), row.name);
        for (const auto& row : importedPacks)
            result.emplace_back(*uuids::uuid::from_string(row.id.value()), row.name);

        return result;
    }

    struct StickerPackWithoutId {
        StickerPackName name;
        UserId ownerId;
    };

    static StickerPackWithoutId get(const StickerPackId& packId) {
        using namespace sqlpp;
        tables::StickerPack sp;
        if (auto result = getDb()(select(all_of(sp)).from(sp).where(sp.id == uuids::to_string(packId)));
            result.size() > 0)
            return StickerPackWithoutId{.name = result.front().name, .ownerId = result.front().ownerId};
        throw std::runtime_error(std::format("StickerPack {} not found", uuids::to_string(packId)));
    }

    static StickerPackName getName(const StickerPackId& packId) {
        using namespace sqlpp;
        tables::StickerPack sp;
        if (auto result = getDb()(select(sp.name).from(sp).where(sp.id == uuids::to_string(packId))); result.size() > 0)
            return result.front().name;
        throw std::runtime_error(std::format("StickerPack {} not found", uuids::to_string(packId)));
    }

    static StickerPackId create(std::string_view name, UserId ownerId) {
        using namespace sqlpp;
        tables::StickerPack sp;
        StickerPackId packId = utils::generateUuid();
        getDb()(insert_into(sp).set(sp.id = uuids::to_string(packId), sp.name = name, sp.ownerId = ownerId));
        return packId;
    }

    static void deletePack(const StickerPackId& packId) {
        using namespace sqlpp;
        tables::StickerPack sp;
        auto packIdStr = uuids::to_string(packId);
        if (getDb()(remove_from(sp).where(sp.id == packIdStr)) == 0)
            throw std::runtime_error(std::format("StickerPack {} not found", packIdStr));
    }
};

} // namespace db
