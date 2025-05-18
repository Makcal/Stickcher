#pragma once

#include "db/db.hpp"
#include "states.hpp"
#include "tables.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <sqlpp11/postgresql/insert.h>
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
               .do_update(s.fileId = state.stickerFileUniqueId));

        tables::Tag t;
        auto tagInsert = insert_into(t).columns(t.packId, t.stickerId, t.text);
        for (const auto& tag : state.tags) {
            tagInsert.values.add(
                t.packId = uuids::to_string(state.packId), t.stickerId = state.stickerFileUniqueId, t.text = tag);
        }
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
        auto rows = db(select(s.fileId, t.text)
                           .from(t.join(s).on(t.stickerId == s.fileUniqueId).join(sp).on(t.packId == sp.id))
                           .where(sp.ownerId == userId));

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
};

} // namespace db
