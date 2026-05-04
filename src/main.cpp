#include <iostream>
#include <signal.h>
#include <sstream>

#include "config/config.h"
#include "log/logger.h"
#include "net/event_loop.h"
#include "net/http_server.h"
#include "router/router.h"
#include "middleware/middleware.h"
#include "middleware/builtin_mw.h"
#include "dao/mysql_pool.h"
#include "dao/database.h"
#include "auth/jwt.h"
#include "auth/password.h"
#include "auth/auth_mw.h"

using namespace dune;

static EventLoop* g_loop = nullptr;

static void on_signal(int sig) {
    LOG_INFO("Received signal " + std::to_string(sig) + ", stopping...");
    if (g_loop) g_loop->stop();
}

// ====== 工具函数 ======

static std::string json_extract(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\":";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size();
    while (pos < json.size() && json[pos] == ' ') pos++;
    if (pos >= json.size()) return "";
    if (json[pos] == '"') {
        auto start = pos + 1;
        auto end = json.find('"', start);
        while (end != std::string::npos && json[end - 1] == '\\')
            end = json.find('"', end + 1);
        return (end != std::string::npos) ? json.substr(start, end - start) : "";
    }
    auto end = json.find_first_of(",}", pos);
    return (end != std::string::npos) ? json.substr(pos, end - pos) : json.substr(pos);
}

static std::string json_escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

static std::string rows_to_json(const ResultSet& rows) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < rows.size(); i++) {
        if (i > 0) oss << ",";
        oss << "{";
        int k = 0;
        for (const auto& kv : rows[i]) {
            if (k++ > 0) oss << ",";
            oss << "\"" << kv.first << "\":\"" << json_escape(kv.second) << "\"";
        }
        oss << "}";
    }
    oss << "]";
    return oss.str();
}

// 生成 JWT payload JSON
static std::string make_jwt_payload(const std::string& user_id) {
    auto now = std::chrono::system_clock::now();
    auto ts  = std::chrono::duration_cast<std::chrono::seconds>(
                   now.time_since_epoch()).count();
    auto exp = ts + 86400; // 24 小时过期
    return R"({"sub":")" + user_id + R"(","iat":)" + std::to_string(ts)
           + R"(,"exp":)" + std::to_string(exp) + "}";
}

// ====== Main ======

int main(int argc, char* argv[]) {
    const char* conf_path = (argc > 1) ? argv[1] : "conf/dune.json";

    Config conf;
    if (!conf.load(conf_path)) {
        std::cerr << "Failed to load config: " << conf_path << std::endl;
        return 1;
    }

    // 日志
    std::string log_level_str = conf.get<std::string>("log.level", "INFO");
    std::string log_file      = conf.get<std::string>("log.file", "logs/dune.log");
    LogLevel min_level = LogLevel::DEBUG;
    if (log_level_str == "FATAL") min_level = LogLevel::FATAL;
    else if (log_level_str == "ERROR") min_level = LogLevel::ERROR;
    else if (log_level_str == "WARN")  min_level = LogLevel::WARN;
    else if (log_level_str == "INFO")  min_level = LogLevel::INFO;
    Logger::instance().init(log_file, min_level);

    // 服务器配置
    std::string host = conf.get<std::string>("server.host", "0.0.0.0");
    int port         = conf.get<int>("server.port", 8080);

    // JWT 密钥
    std::string jwt_secret = conf.get<std::string>("auth.jwt_secret", "dune-secret-change-me");

    LOG_INFO("========== Dune Server Starting ==========");

    // 数据库
    MysqlConfig db_cfg;
    db_cfg.host     = conf.get<std::string>("database.host", "127.0.0.1");
    db_cfg.port     = conf.get<int>("database.port", 3306);
    db_cfg.user     = conf.get<std::string>("database.user", "root");
    db_cfg.password = conf.get<std::string>("database.password", "");
    db_cfg.dbname   = conf.get<std::string>("database.dbname", "dune");
    db_cfg.pool_size = conf.get<int>("database.pool_size", 4);

    auto pool = std::make_shared<MysqlPool>(db_cfg);
    auto db   = std::make_shared<Database>(pool);
    SetDB(db.get());

    // 测试连接
    {
        auto rows = db->query("SELECT COUNT(*) AS cnt FROM articles");
        if (!rows.empty())
            LOG_INFO("Database connected, articles: " + rows[0]["cnt"]);
    }

    // ====== 路由 ======
    Router router;

    router.add("GET", "/", [](HttpRequest& req, HttpResponse& resp) {
        resp.write(200, R"({"code":0,"msg":"welcome to Dune"})");
    });

    router.add("GET", "/health", [](HttpRequest& req, HttpResponse& resp) {
        resp.write(200, R"({"status":"ok"})");
    });

    // ---------- 认证 API（无认证） ----------

    // 注册
    router.add("POST", "/api/auth/register",
        [&db, &jwt_secret](HttpRequest& req, HttpResponse& resp) {
            std::string username = json_extract(req.body, "username");
            std::string pass     = json_extract(req.body, "password");
            std::string nickname = json_extract(req.body, "nickname");

            if (username.empty() || pass.empty()) {
                resp.write(400, R"({"code":400,"msg":"username and password required"})");
                return;
            }
            if (username.size() > 64 || pass.size() > 64) {
                resp.write(400, R"({"code":400,"msg":"username or password too long"})");
                return;
            }

            // 检查用户名是否存在
            auto existing = db->query(
                "SELECT id FROM users WHERE username='"
                + db->escape(username) + "'");
            if (!existing.empty()) {
                resp.write(409, R"({"code":409,"msg":"username already exists"})");
                return;
            }

            std::string hashed = password::hash(pass);
            std::string sql =
                "INSERT INTO users (username, password, nickname) VALUES ('"
                + db->escape(username) + "','" + db->escape(hashed) + "','"
                + db->escape(nickname) + "')";
            int64_t uid = db->insert(sql);
            if (uid < 0) {
                resp.write(500, R"({"code":500,"msg":"register failed"})");
                return;
            }

            // 注册成功，直接签发 token
            std::string token = jwt::encode(
                make_jwt_payload(std::to_string(uid)), jwt_secret);

            resp.write(201,
                R"({"code":0,"msg":"registered","data":{"token":")"
                + token + R"(","user_id":)" + std::to_string(uid) + "}}");
        });

    // 登录
    router.add("POST", "/api/auth/login",
        [&db, &jwt_secret](HttpRequest& req, HttpResponse& resp) {
            std::string username = json_extract(req.body, "username");
            std::string pass     = json_extract(req.body, "password");

            if (username.empty() || pass.empty()) {
                resp.write(400, R"({"code":400,"msg":"username and password required"})");
                return;
            }

            auto rows = db->query(
                "SELECT id, password FROM users WHERE username='"
                + db->escape(username) + "'");
            if (rows.empty()) {
                resp.write(401, R"({"code":401,"msg":"invalid username or password"})");
                return;
            }

            std::string stored_hash = rows[0]["password"];
            if (!password::verify(pass, stored_hash)) {
                resp.write(401, R"({"code":401,"msg":"invalid username or password"})");
                return;
            }

            std::string uid   = rows[0]["id"];
            std::string token = jwt::encode(make_jwt_payload(uid), jwt_secret);

            resp.write(200,
                R"({"code":0,"msg":"ok","data":{"token":")"
                + token + R"(","user_id":)" + uid + "}}");
        });

    // ---------- 需要认证的 API ----------

    // 当前用户信息
    router.add("GET", "/api/auth/me",
        [&db](HttpRequest& req, HttpResponse& resp) {
            std::string uid = req.param("_user_id");
            auto rows = db->query(
                "SELECT id, username, nickname, created_at FROM users WHERE id="
                + db->escape(uid));
            if (rows.empty()) {
                resp.write(404, R"({"code":404,"msg":"user not found"})");
                return;
            }
            resp.write(200, R"({"code":0,"data":)" + rows_to_json(rows) + "}");
        });

    // ---------- 文章 API ----------
    router.add("GET", "/api/articles",
        [&db](HttpRequest& req, HttpResponse& resp) {
            auto rows = db->query(
                "SELECT id, title, created_at FROM articles ORDER BY id DESC");
            resp.write(200, R"({"code":0,"data":)" + rows_to_json(rows) + "}");
        });

    router.add("GET", "/api/articles/:id",
        [&db](HttpRequest& req, HttpResponse& resp) {
            std::string id = db->escape(req.param("id", "0"));
            auto rows = db->query(
                "SELECT id, title, content, created_at FROM articles WHERE id=" + id);
            if (rows.empty()) {
                resp.write(404, R"({"code":404,"msg":"article not found"})");
                return;
            }
            resp.write(200, R"({"code":0,"data":)" + rows_to_json(rows) + "}");
        });

    // 创建文章（需要认证）
    router.add("POST", "/api/articles",
        [&db](HttpRequest& req, HttpResponse& resp) {
            std::string title   = db->escape(json_extract(req.body, "title"));
            std::string content = db->escape(json_extract(req.body, "content"));

            if (title.empty()) {
                resp.write(400, R"({"code":400,"msg":"title is required"})");
                return;
            }
            std::string sql =
                "INSERT INTO articles (title, content) VALUES ('"
                + title + "', '" + content + "')";
            int64_t new_id = db->insert(sql);
            if (new_id < 0) {
                resp.write(500, R"({"code":500,"msg":"insert failed"})");
                return;
            }
            resp.write(201,
                R"({"code":0,"msg":"created","data":{"id":)"
                + std::to_string(new_id) + "}}");
        });

    router.add("DELETE", "/api/articles/:id",
        [&db](HttpRequest& req, HttpResponse& resp) {
            std::string id = db->escape(req.param("id", "0"));
            int ret = db->execute("DELETE FROM articles WHERE id=" + id);
            if (ret <= 0) {
                resp.write(404, R"({"code":404,"msg":"article not found"})");
                return;
            }
            resp.write(200, R"({"code":0,"msg":"deleted"})");
        });

    router.add("GET", "/api/search",
        [&db](HttpRequest& req, HttpResponse& resp) {
            std::string q = db->escape(req.param("q", ""));
            if (q.empty()) {
                resp.write(400, R"({"code":400,"msg":"missing 'q' param"})");
                return;
            }
            auto rows = db->query(
                "SELECT id, title, created_at FROM articles "
                "WHERE title LIKE '%" + q + "%' OR content LIKE '%" + q + "%'");
            resp.write(200, R"({"code":0,"data":)" + rows_to_json(rows) + "}");
        });

    LOG_INFO("Routes registered: " + std::to_string(router.size()));

    // ====== 中间件链 ======
    MiddlewareChain chain;

    chain.add(std::make_unique<RecoveryMiddleware>());
    chain.add(std::make_unique<LoggerMiddleware>());
    chain.add(std::make_unique<CorsMiddleware>());
    // Auth 中间件：需要认证的接口才会拦截，其他 skip 白名单放行
    auto auth_mw = std::make_unique<AuthMiddleware>(jwt_secret);
    auth_mw->skip("/api/auth/login")
           .skip("/api/auth/register")
           .skip("/")
           .skip("/health")
           .skip("/api/search")
           .skip("/api/articles");
    chain.add(std::move(auth_mw));

    chain.set_handler([&router](HttpRequest& req, HttpResponse& resp) {
        auto handler = router.dispatch(req);
        if (handler) {
            handler(req, resp);
        } else {
            resp.write(404, R"({"code":404,"msg":"not found"})");
        }
    });

    // ====== 启动 ======
    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);

    EventLoop loop;
    g_loop = &loop;

    HttpServer server(loop, host, port);
    server.set_handler([&chain](HttpRequest& req, HttpResponse& resp) {
        chain.run(req, resp);
    });

    if (server.start() != 0) {
        LOG_FATAL("Failed to start HttpServer");
        return 1;
    }

    int ret = loop.run();
    LOG_INFO("========== Dune Server Stopped ==========");
    return ret;
}
