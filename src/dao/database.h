#ifndef DUNE_DAO_DATABASE_H_
#define DUNE_DAO_DATABASE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "mysql_pool.h"

namespace dune {

// 一行记录: 字段名 → 值
using Row = std::unordered_map<std::string, std::string>;
using ResultSet = std::vector<Row>;

class Database {
public:
    explicit Database(std::shared_ptr<MysqlPool> pool)
        : pool_(std::move(pool)) {}

    // 执行 INSERT/UPDATE/DELETE，返回影响行数，失败返回 -1
    int execute(const std::string& sql);

    // 执行 SELECT，返回结果集
    ResultSet query(const std::string& sql);

    // 字符串转义（防 SQL 注入）
    std::string escape(const std::string& str);

    // 获取底层连接池
    std::shared_ptr<MysqlPool> pool() const { return pool_; }

    // INSERT 并返回自增 id（同连接，保证 id 正确），失败返回 -1
    int64_t insert(const std::string& sql);

    // 便捷：获取最后一个 INSERT 的 id
    int64_t last_insert_id();

private:
    std::shared_ptr<MysqlPool> pool_;
};

// 全局单例（初始化后设置）
Database* DB();
void SetDB(Database* db);

} // namespace dune

#endif // DUNE_DAO_DATABASE_H_
