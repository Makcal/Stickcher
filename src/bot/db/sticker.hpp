#pragma once

#include "db/db.hpp"
#include "states.hpp"
#include "tables.hpp"

#include <sqlpp11/insert.h>
#include <uuid.h>

namespace db {

class StickerRepository {
  public:
    static void create(const states::TagAddition& state) {
        using namespace sqlpp;
        auto db = getDb();

        tables::Sticker s;
        db(insert_into(s).set(s.fileId = state.stickerFileId, s.fileUniqueId = state.stickerFileUniqueId));

        tables::Association as;
        auto associationInsert = insert_into(as).columns(as.packId, as.stickerId, as.text);
        for (const auto& tag : state.tags) {
            associationInsert.values.add(
                as.packId = uuids::to_string(state.packId), as.stickerId = state.stickerFileUniqueId, as.text = tag);
        }
        db(associationInsert);
    }
};

} // namespace db
