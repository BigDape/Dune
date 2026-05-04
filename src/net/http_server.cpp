#include "http_server.h"
#include "event_loop.h"
#include "log/logger.h"
#include <cstring>
#include <sstream>

namespace dune {

// 解析 query string → map
static void parse_query(const std::string& qs,
                        std::unordered_map<std::string, std::string>& out) {
    if (qs.empty()) return;
    std::istringstream ss(qs);
    std::string kv;
    while (std::getline(ss, kv, '&')) {
        auto pos = kv.find('=');
        if (pos != std::string::npos) {
            out[kv.substr(0, pos)] = kv.substr(pos + 1);
        } else {
            out[kv] = "";
        }
    }
}

HttpServer::HttpServer(EventLoop& loop, const std::string& host, int port)
    : loop_(loop), host_(host), port_(port) {
}

HttpServer::~HttpServer() {
    if (http_) {
        evhttp_free(http_);
        http_ = nullptr;
        LOG_DEBUG("HttpServer destroyed");
    }
}

int HttpServer::start() {
    http_ = evhttp_new(loop_.base());
    if (!http_) {
        LOG_FATAL("Failed to create evhttp");
        return -1;
    }

    // 设置全局请求回调（将 this 通过 arg 传入）
    evhttp_set_gencb(http_, on_request, this);

    // 允许所有常用 HTTP 方法（默认只允许 GET/POST/HEAD/PUT/DELETE）
    evhttp_set_allowed_methods(http_,
        EVHTTP_REQ_GET | EVHTTP_REQ_POST | EVHTTP_REQ_PUT |
        EVHTTP_REQ_DELETE | EVHTTP_REQ_OPTIONS | EVHTTP_REQ_PATCH |
        EVHTTP_REQ_HEAD);

    // 绑定端口
    int ret = evhttp_bind_socket(http_, host_.c_str(), port_);
    if (ret != 0) {
        LOG_FATAL("Failed to bind " + host_ + ":" + std::to_string(port_));
        return -1;
    }

    LOG_INFO("HttpServer listening on http://" + host_ + ":" + std::to_string(port_));
    return 0;
}

void HttpServer::stop() {
    LOG_INFO("HttpServer stopping...");
}

void HttpServer::set_handler(HttpHandler handler) {
    handler_ = std::move(handler);
}

void HttpServer::on_request(evhttp_request* req, void* arg) {
    auto* server = static_cast<HttpServer*>(arg);

    // 解析请求
    const char* uri = evhttp_request_get_uri(req);
    evhttp_cmd_type cmd = evhttp_request_get_command(req);

    std::string full_uri = uri ? uri : "/";

    HttpRequest hreq;
    hreq.ev_req = req;

    // 拆分 path 和 query string
    auto qpos = full_uri.find('?');
    if (qpos != std::string::npos) {
        hreq.path  = full_uri.substr(0, qpos);
        hreq.query = full_uri.substr(qpos + 1);
        parse_query(hreq.query, hreq.queries);
    } else {
        hreq.path = full_uri;
    }
    hreq.method = (cmd == EVHTTP_REQ_GET)     ? "GET"
                : (cmd == EVHTTP_REQ_POST)    ? "POST"
                : (cmd == EVHTTP_REQ_PUT)     ? "PUT"
                : (cmd == EVHTTP_REQ_DELETE)  ? "DELETE"
                : (cmd == EVHTTP_REQ_OPTIONS) ? "OPTIONS"
                : (cmd == EVHTTP_REQ_PATCH)   ? "PATCH"
                : (cmd == EVHTTP_REQ_HEAD)    ? "HEAD"
                :                               "UNKNOWN";

    // 读取 body
    auto* inbuf = evhttp_request_get_input_buffer(req);
    size_t len = evbuffer_get_length(inbuf);
    if (len > 0) {
        hreq.body.assign(reinterpret_cast<const char*>(
            evbuffer_pullup(inbuf, len)), len);
    }

    LOG_INFO(hreq.method + " " + hreq.path);

    HttpResponse resp(req);

    if (server->handler_) {
        server->handler_(hreq, resp);
    } else {
        // 默认响应
        resp.write(200, R"({"msg":"Dune server is running"})");
    }
}

} // namespace dune
