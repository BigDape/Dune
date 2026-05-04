#include "mysql_pool.h"
#include "log/logger.h"
#include <thread>
#include <chrono>

namespace dune {

MysqlPool::MysqlPool(const MysqlConfig& cfg)
    : cfg_(cfg), pool_size_(cfg.pool_size) {
    LOG_INFO("MysqlPool init: " + cfg_.host + ":" + std::to_string(cfg_.port)
             + "/" + cfg_.dbname + "  pool_size=" + std::to_string(pool_size_));
}

MysqlPool::~MysqlPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!pool_.empty()) {
        destroy_conn(pool_.front());
        pool_.pop();
    }
    LOG_INFO("MysqlPool destroyed");
}

MYSQL* MysqlPool::create_conn() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        LOG_ERROR("mysql_init failed");
        return nullptr;
    }

    // 设置自动重连
    bool reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);

    // 设置连接超时
    unsigned int timeout = 3;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    if (!mysql_real_connect(conn, cfg_.host.c_str(), cfg_.user.c_str(),
                            cfg_.password.c_str(), cfg_.dbname.c_str(),
                            cfg_.port, nullptr, 0)) {
        LOG_ERROR("mysql_real_connect failed: " + std::string(mysql_error(conn)));
        mysql_close(conn);
        return nullptr;
    }

    mysql_set_character_set(conn, "utf8mb4");
    return conn;
}

void MysqlPool::destroy_conn(MYSQL* conn) {
    if (conn) mysql_close(conn);
}

bool MysqlPool::ping(MYSQL* conn) {
    return mysql_ping(conn) == 0;
}

MYSQL* MysqlPool::acquire() {
    std::unique_lock<std::mutex> lock(mutex_);

    // 等待可用连接
    while (pool_.empty()) {
        // 尝试创建新连接
        lock.unlock();
        MYSQL* conn = create_conn();
        if (conn) return conn;

        // 创建失败，等待归还
        lock.lock();
        if (cv_.wait_for(lock, std::chrono::seconds(3)) == std::cv_status::timeout) {
            LOG_ERROR("MysqlPool acquire timeout");
            return nullptr;
        }
    }

    MYSQL* conn = pool_.front();
    pool_.pop();

    // 心跳检查，不通过则重建
    if (!ping(conn)) {
        LOG_WARN("MySQL connection lost, reconnecting...");
        destroy_conn(conn);
        conn = create_conn();
    }

    return conn;
}

void MysqlPool::release(MYSQL* conn) {
    if (!conn) return;
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.push(conn);
    cv_.notify_one();
}

int MysqlPool::idle_size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(pool_.size());
}

} // namespace dune
