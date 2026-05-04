#ifndef DUNE_DAO_MYSQL_POOL_H_
#define DUNE_DAO_MYSQL_POOL_H_

#include <string>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <mysql/mysql.h>

namespace dune {

struct MysqlConfig {
    std::string host     = "127.0.0.1";
    int         port     = 3306;
    std::string user     = "root";
    std::string password = "";
    std::string dbname   = "dune";
    int         pool_size = 8;
};

class MysqlPool {
public:
    explicit MysqlPool(const MysqlConfig& cfg);
    ~MysqlPool();

    MysqlPool(const MysqlPool&) = delete;
    MysqlPool& operator=(const MysqlPool&) = delete;

    // 获取一个连接（使用完必须调用 release 归还）
    MYSQL* acquire();

    // 归还连接
    void release(MYSQL* conn);

    int  pool_size() const { return pool_size_; }
    int  idle_size();

private:
    MYSQL* create_conn();
    void   destroy_conn(MYSQL* conn);
    bool   ping(MYSQL* conn);

    MysqlConfig cfg_;
    int         pool_size_ = 8;

    std::queue<MYSQL*> pool_;
    std::mutex         mutex_;
    std::condition_variable cv_;
};

// RAII 连接自动归还
class ScopedConn {
public:
    explicit ScopedConn(MysqlPool& pool) : pool_(pool), conn_(pool.acquire()) {}
    ~ScopedConn() { pool_.release(conn_); }

    ScopedConn(const ScopedConn&) = delete;
    ScopedConn& operator=(const ScopedConn&) = delete;

    MYSQL* operator->() { return conn_; }
    MYSQL* get()        { return conn_; }

private:
    MysqlPool& pool_;
    MYSQL*     conn_;
};

} // namespace dune

#endif // DUNE_DAO_MYSQL_POOL_H_
