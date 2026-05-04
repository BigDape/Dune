#include "builtin_mw.h"
#include "net/http_server.h"
#include "log/logger.h"
#include <chrono>

namespace dune {

// ===================== LoggerMiddleware =====================
void LoggerMiddleware::process(HttpRequest& req, HttpResponse& resp, Next next) {
    auto t0 = std::chrono::steady_clock::now();

    // 前置：记录请求
    LOG_INFO("--> " + req.method + " " + req.path);

    next();

    // 后置：记录耗时
    auto t1 = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    LOG_INFO("<-- " + req.method + " " + req.path
             + "  cost=" + std::to_string(ms) + "us");
}

// ===================== CorsMiddleware =====================
void CorsMiddleware::process(HttpRequest& req, HttpResponse& resp, Next next) {
    // 前置：设置 CORS 头
    resp.set_header("Access-Control-Allow-Origin", "*");
    resp.set_header("Access-Control-Allow-Methods",
                    "GET, POST, PUT, DELETE, OPTIONS");
    resp.set_header("Access-Control-Allow-Headers",
                    "Content-Type, Authorization");

    // OPTIONS 预检直接返回 204
    if (req.method == "OPTIONS") {
        resp.write(204, "");
        return;
    }

    next();
}

// ===================== RecoveryMiddleware =====================
void RecoveryMiddleware::process(HttpRequest& req, HttpResponse& resp, Next next) {
    try {
        next();
    } catch (const std::exception& e) {
        LOG_ERROR("Recovery caught exception: " + std::string(e.what()));
        resp.write(500, R"({"code":500,"msg":"Internal Server Error"})");
    } catch (...) {
        LOG_ERROR("Recovery caught unknown exception");
        resp.write(500, R"({"code":500,"msg":"Internal Server Error"})");
    }
}

} // namespace dune
