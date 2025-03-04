#include <print>

#include <sqlite_orm/sqlite_orm.h>

#include "db.hpp"

int main() {
    auto db = get_db();
    db.sync_schema();

    for (const auto& u : db.get_all<Sticker>()) {
        std::println("{}: {}", u.file_unique_id, u.file_id);
    }
}
