#pragma once

#include "db/db.hpp"
#include "states.hpp"
#include "tables.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <sqlpp11/postgresql/insert.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/select.h>
#include <uuid.h>

#include <cstddef>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace db {

class StickerRepository {
  public:
    static void create(const states::TagAddition& state) {
        using namespace sqlpp;
        auto db = getDb();

        tables::Sticker s;
        db(postgresql::insert_into(s)
               .set(s.fileId = state.stickerFileId, s.fileUniqueId = state.stickerFileUniqueId)
               .on_conflict(s.fileUniqueId)
               .do_update(s.fileId = state.stickerFileId));

        tables::Tag t;
        auto packIdStr = uuids::to_string(state.packId);
        db(remove_from(t).where(t.packId == packIdStr && t.stickerId == state.stickerFileUniqueId));
        auto tagInsert = insert_into(t).columns(t.packId, t.stickerId, t.text);
        for (const auto& tag : state.tags)
            tagInsert.values.add(t.packId = packIdStr, t.stickerId = state.stickerFileUniqueId, t.text = tag);
        db(tagInsert);
    }

    static std::vector<StickerFileId>
    findAllByTag(UserId userId, std::string_view tag, double similarityThreshold, std::size_t limit) {
        using namespace sqlpp;
        using namespace std::ranges;
        auto db = getDb();

        tables::Tag t;
        tables::Sticker s;
        tables::StickerPack sp;
        tables::PackSharing ps;
        auto selectOwn = select(s.fileId, t.text)
                             .from(t.join(s).on(t.stickerId == s.fileUniqueId).join(sp).on(t.packId == sp.id))
                             .where(sp.ownerId == userId);
        auto selectImported = select(s.fileId, t.text)
                                  .from(t.join(s).on(t.stickerId == s.fileUniqueId).join(ps).on(t.packId == ps.packId))
                                  .where(ps.userId == userId);
        auto rows = db(selectOwn.union_all(selectImported));

        using Association = std::pair<StickerFileId, std::string>;
        std::vector<Association> associations;
        for (const auto& row : rows)
            associations.emplace_back(row.fileId, row.text);

        auto ratios = utils::ratioToAll(associations, tag, similarityThreshold, &Association::second);
        sort(ratios, greater{}, [](auto& r) { return r.second; });
        return ratios | views::take(limit) |
               views::transform([](auto& r) -> decltype(auto) { return std::move(r.first->first); }) |
               to<std::vector>();
    }

    static bool deleteFromPack(const StickerPackId& packId, std::string_view fileUniqueId) {
        using namespace sqlpp;
        tables::Tag t;
        return getDb()(remove_from(t).where(t.stickerId == fileUniqueId && t.packId == uuids::to_string(packId))) != 0;
    }

    static std::vector<std::string> getTags(const StickerPackId& packId, std::string_view fileUniqueId) {
        using namespace sqlpp;
        tables::Tag t;
        auto db = getDb();
        auto tags =
            db(select(t.text).from(t).where(t.stickerId == fileUniqueId && t.packId == uuids::to_string(packId)));
        std::vector<std::string> result;
        result.reserve(tags.size());
        for (const auto& row : tags)
            result.emplace_back(row.text);
        return result;
    }
};

} // namespace db
