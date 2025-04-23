#include <cstdint>
#include <sqlite_orm/sqlite_orm.h>
#include <string>

struct Sticker {
    std::string file_unique_id;
    std::string file_id;
};

// Not same as Telegram's packs
struct StickerPack {
    std::uint64_t id;
    std::string name;
    std::uint64_t owner_id;
};

struct StickerAssociation {
    std::uint64_t id;
    std::string text;
    decltype(Sticker::file_unique_id) sticker_id;
    decltype(StickerPack::id) pack_id;
};

/*template <>*/
/*struct sqlite_orm::row_extractor<uuids::uuid> {*/
/*    static uuids::uuid extract(sqlite3_stmt* stmt, int index) {*/
/*        // NOLINTNEXTLINE(*-reinterpret-cast)*/
/*        const auto* columnText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));*/
/*        auto mUuid = uuids::uuid::from_string(columnText);*/
/*        if (mUuid) {*/
/*            return mUuid.value();*/
/*        }*/
/*        throw std::runtime_error{std::format("Bad UUID: {}", columnText)};*/
/*    }*/
/*};*/

inline auto get_db() {
    using namespace sqlite_orm;
    return make_storage("db.sqlite",
                        make_table("stickers",
                                   make_column("file_unique_id", &Sticker::file_unique_id, primary_key()),
                                   make_column("file_id", &Sticker::file_id)),
                        make_table("sticker_packs",
                                   make_column("id", &StickerPack::id, primary_key()),
                                   make_column("name", &StickerPack::name),
                                   make_column("owner_id", &StickerPack::owner_id)),
                        make_table("associations",
                                   make_column("id", &StickerAssociation::id, primary_key().autoincrement()),
                                   make_column("text", &StickerAssociation::text),
                                   make_column("sticker_id", &StickerAssociation::sticker_id),
                                   make_column("pack_id", &StickerAssociation::pack_id),
                                   foreign_key(&StickerAssociation::sticker_id).references(&Sticker::file_unique_id),
                                   foreign_key(&StickerAssociation::pack_id).references(&StickerPack::id)));
}
