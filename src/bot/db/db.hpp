#pragma once

#include "utils.hpp"

#include <sqlpp11/postgresql/connection.h>
#include <sqlpp11/postgresql/connection_config.h>

namespace db {

class DbConfig : public sqlpp::postgresql::connection_config {
  public:
    DbConfig() : connection_config{} {
        host = utils::getenvWithError("DB_HOST");
        user = utils::getenvWithError("DB_USER");
        password = utils::getenvWithError("DB_PASSWORD");
        dbname = utils::getenvWithError("DB_NAME");
    }
};

using DbConnection = sqlpp::postgresql::connection;

inline DbConnection getDb() {
    static const DbConfig config{};
    return sqlpp::postgresql::connection{config};
}

} // namespace db
