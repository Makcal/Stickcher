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

        tables::Tag t;
        auto tagInsert = insert_into(t).columns(t.packId, t.stickerId, t.text);
        for (const auto& tag : state.tags) {
            tagInsert.values.add(
                t.packId = uuids::to_string(state.packId), t.stickerId = state.stickerFileUniqueId, t.text = tag);
        }
        db(tagInsert);
    }
};

} // namespace db
