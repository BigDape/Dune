#include "database.h"
#include "log/logger.h"
#include <cstring>

namespace dune {

static Database* g_db = nullptr;

Database* DB() { return g_db; }
void SetDB(Database* db) { g_db = db; }

int Database::execute(const std::string& sql) {
    ScopedConn conn(*pool_);
    if (!conn.get()) return -1;

    if (mysql_query(conn.get(), sql.c_str()) != 0) {
        LOG_ERROR("SQL execute error: " + std::string(mysql_error(conn.get()))
                  + "  sql=" + sql);
        return -1;
    }

    int affected = static_cast<int>(mysql_affected_rows(conn.get()));
    LOG_DEBUG("SQL ok, affected=" + std::to_string(affected));
    return affected;
}

ResultSet Database::query(const std::string& sql) {
    ResultSet results;
    ScopedConn conn(*pool_);
    if (!conn.get()) return results;

    if (mysql_query(conn.get(), sql.c_str()) != 0) {
        LOG_ERROR("SQL query error: " + std::string(mysql_error(conn.get()))
                  + "  sql=" + sql);
        return results;
    }

    MYSQL_RES* res = mysql_store_result(conn.get());
    if (!res) return results;

    unsigned int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Row r;
        unsigned long* lengths = mysql_fetch_lengths(res);
        for (unsigned int i = 0; i < num_fields; i++) {
            r[fields[i].name] = (row[i] && lengths[i] > 0)
                                ? std::string(row[i], lengths[i]) : "";
        }
        results.push_back(std::move(r));
    }

    mysql_free_result(res);
    LOG_DEBUG("SQL query ok, rows=" + std::to_string(results.size()));
    return results;
}

std::string Database::escape(const std::string& str) {
    ScopedConn conn(*pool_);
    if (!conn.get()) return "";

    std::string out(str.size() * 2 + 1, '\0');
    mysql_real_escape_string(conn.get(), &out[0], str.c_str(), str.size());
    out.resize(std::strlen(out.c_str()));
    return out;
}

int64_t Database::last_insert_id() {
    ScopedConn conn(*pool_);
    if (!conn.get()) return -1;
    return static_cast<int64_t>(mysql_insert_id(conn.get()));
}

int64_t Database::insert(const std::string& sql) {
    ScopedConn conn(*pool_);
    if (!conn.get()) return -1;

    if (mysql_query(conn.get(), sql.c_str()) != 0) {
        LOG_ERROR("SQL insert error: " + std::string(mysql_error(conn.get()))
                  + "  sql=" + sql);
        return -1;
    }

    int64_t id = static_cast<int64_t>(mysql_insert_id(conn.get()));
    LOG_DEBUG("SQL insert ok, id=" + std::to_string(id));
    return id;
}

} // namespace dune
