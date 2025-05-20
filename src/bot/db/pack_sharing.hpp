#pragma once

#include "db/db.hpp"
#include "db/models.hpp"
#include "tables.hpp"
#include "types.hpp"

#include <sqlpp11/insert.h>
#include <sqlpp11/postgresql/exception.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/select.h>
#include <uuid.h>

#include <format>
#include <stdexcept>
#include <vector>

namespace db {

using namespace models;

class PackSharingRepository {
  public:
    enum class ImportResult : char {
        Success,
        NotExist,
        ImportByOwner,
        AlreadyImported,
    };

    static ImportResult import(const StickerPackId& packId, UserId userId) {
        using namespace sqlpp;
        auto db = getDb();
        auto packIdStr = uuids::to_string(packId);

        tables::StickerPack sp;
        const auto& row = db(select(sp.ownerId).from(sp).where(sp.id == packIdStr)).front();
        if (!row)
            return ImportResult::NotExist;
        if (row.ownerId == userId)
            return ImportResult::ImportByOwner;

        try {
            tables::PackSharing ps;
            db(insert_into(ps).set(ps.userId = userId, ps.packId = packIdStr));
            return ImportResult::Success;
        } catch (postgresql::unique_violation&) {
            return ImportResult::AlreadyImported;
        }
    }

    static bool checkEditorRights(const StickerPackId& packId, UserId userId) {
        using namespace sqlpp;
        tables::PackSharing ps;
        auto query = select(ps.isEditor).from(ps).where(ps.packId == uuids::to_string(packId) && ps.userId == userId);
        if (const auto& row = getDb()(query).front())
            return row.isEditor;
        return false;
    }

    static void remove(UserId userId, const StickerPackId& packId) {
        using namespace sqlpp;
        tables::PackSharing ps;
        auto packIdStr = uuids::to_string(packId);
        if (getDb()(remove_from(ps).where(ps.userId == userId && ps.packId == packIdStr)) == 0)
            throw std::runtime_error(std::format("User {} didn't import {}", userId, packIdStr));
    }

    static bool flipIsEditor(const StickerPackId& packId, UserId userId) {
        using namespace sqlpp;
        tables::PackSharing ps;
        auto query = update(ps)
                         .set(ps.isEditor = !ps.isEditor)
                         .where(ps.userId == userId && ps.packId == uuids::to_string(packId));
        return getDb()(query) != 0;
    }

    static std::vector<UserId> getEditors(const StickerPackId& packId) {
        using namespace sqlpp;
        auto db = getDb();
        tables::PackSharing ps;
        auto ids = db(select(ps.userId).from(ps).where(ps.packId == uuids::to_string(packId) && ps.isEditor == true));
        std::vector<UserId> results;
        results.reserve(ids.size());
        for (const auto& row : ids)
            results.push_back(row.userId);
        return results;
    }
};

} // namespace db

