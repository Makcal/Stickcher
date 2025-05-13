#pragma once

#include "db/db.hpp"
#include "db/models.hpp"
#include "tables.hpp"
#include "types.hpp"

#include <sqlpp11/select.h>

#include <uuid.h>
#include <vector>

namespace db {

using namespace db::models;

class StickerPackRepository {
  public:
    struct UsersStickerPack {
        decltype(StickerPack::id) id;
        decltype(StickerPack::name) name;
    };

    static std::vector<UsersStickerPack> getUserPacks(UserId ownerId) {
        auto db = getDb();
        tables::StickerPack sp;
        auto packs = db(sqlpp::select(sp.id, sp.name).from(sp).where(sp.ownerId == ownerId));

        std::vector<UsersStickerPack> result;
        result.reserve(packs.size());
        for (const auto& row : packs) {
            result.push_back(UsersStickerPack{.id = *uuids::uuid::from_string(row.id.value()), .name = row.name});
        }
        return result;
    }
};

} // namespace db
