#include "auth_mw.h"
#include "net/http_server.h"
#include "auth/jwt.h"
#include "log/logger.h"

namespace dune {

AuthMiddleware& AuthMiddleware::skip(const std::string& prefix) {
    skip_prefixes_.push_back(prefix);
    return *this;
}

void AuthMiddleware::process(HttpRequest& req, HttpResponse& resp, Next next) {
    LOG_INFO("AuthMW inspecting: " + req.path);

    // 跳过白名单路径
    // 匹配规则: 前缀完全相等，或 路径以 前缀/ 开头（保证前缀是完整路径段）
    for (const auto& prefix : skip_prefixes_) {
        size_t plen = prefix.size();
        if (req.path.size() >= plen &&
            req.path.compare(0, plen, prefix) == 0 &&
            (req.path.size() == plen || req.path[plen] == '/')) {
            LOG_INFO("AuthMW skip: " + prefix);
            next();
            return;
        }
    }

    // 提取 Authorization 头
    const char* auth_header = evhttp_find_header(
        evhttp_request_get_input_headers(req.ev_req), "Authorization");
    if (!auth_header) {
        LOG_INFO("AuthMW reject: missing Authorization header");
        resp.write(401, R"({"code":401,"msg":"missing Authorization header"})");
        return;
    }

    std::string auth(auth_header);
    const std::string prefix = "Bearer ";
    if (auth.size() < prefix.size() || auth.substr(0, prefix.size()) != prefix) {
        resp.write(401, R"({"code":401,"msg":"invalid Authorization format"})");
        return;
    }

    std::string token = auth.substr(prefix.size());
    std::string payload = jwt::decode(token, secret_);
    if (payload.empty()) {
        LOG_WARN("JWT verify failed, token expired or invalid");
        resp.write(401, R"({"code":401,"msg":"invalid or expired token"})");
        return;
    }

    // 简单提取 sub 字段（user_id）
    auto pos = payload.find("\"sub\":\"");
    if (pos != std::string::npos) {
        auto start = pos + 7;
        auto end = payload.find('"', start);
        if (end != std::string::npos) {
            req.params["_user_id"] = payload.substr(start, end - start);
        }
    }
    req.params["_payload"] = payload;

    LOG_INFO("Auth ok, user_id=" + req.params["_user_id"]);
    next();
}

} // namespace dune
