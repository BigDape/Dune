#ifndef DUNE_NET_HTTP_SERVER_H_
#define DUNE_NET_HTTP_SERVER_H_

#include <string>
#include <unordered_map>
#include <functional>
#include <event2/http.h>
#include <event2/buffer.h>

namespace dune {

class EventLoop;

// 请求上下文
struct HttpRequest {
    evhttp_request* ev_req = nullptr;
    std::string method;
    std::string path;   // 不含 query string 的路径
    std::string query;  // 原始 query string
    std::string body;

    // 路由参数 (:id → "42") 和查询参数 (?page=1&size=10)
    std::unordered_map<std::string, std::string> params;
    std::unordered_map<std::string, std::string> queries;

    // 获取参数（优先路由参数，其次查询参数）
    std::string param(const std::string& key,
                      const std::string& default_val = "") const {
        auto it = params.find(key);
        if (it != params.end()) return it->second;
        auto jt = queries.find(key);
        return (jt != queries.end()) ? jt->second : default_val;
    }
};

// 响应对象
class HttpResponse {
public:
    explicit HttpResponse(evhttp_request* req) : ev_req_(req) {}

    void set_header(const std::string& key, const std::string& val) {
        evhttp_add_header(
            evhttp_request_get_output_headers(ev_req_),
            key.c_str(), val.c_str());
    }

    void write(int code, const std::string& body,
               const std::string& content_type = "application/json") {
        set_header("Content-Type", content_type);
        auto* buf = evhttp_request_get_output_buffer(ev_req_);
        evbuffer_add(buf, body.c_str(), body.size());
        evhttp_send_reply(ev_req_, code, nullptr, nullptr);
    }

private:
    evhttp_request* ev_req_;
};

// HTTP 请求处理器签名
using HttpHandler = std::function<void(HttpRequest&, HttpResponse&)>;

class HttpServer {
public:
    HttpServer(EventLoop& loop, const std::string& host, int port);
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    // 注册全局处理器（后续 Router 模块会替代此接口）
    void set_handler(HttpHandler handler);

    // 启动 / 停止
    int start();
    void stop();

private:
    static void on_request(evhttp_request* req, void* arg);

    EventLoop&       loop_;
    std::string       host_;
    int               port_ = 8080;
    evhttp*           http_ = nullptr;
    HttpHandler       handler_;
};

} // namespace dune

#endif // DUNE_NET_HTTP_SERVER_H_
