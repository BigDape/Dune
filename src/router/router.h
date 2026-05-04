#ifndef DUNE_ROUTER_ROUTER_H_
#define DUNE_ROUTER_ROUTER_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace dune {

struct HttpRequest;
class HttpResponse;
using RouteHandler = std::function<void(HttpRequest&, HttpResponse&)>;

// 前缀树节点
class RouteNode {
public:
    // 注册路由，返回该节点上的 handler 表引用
    void add_handler(const std::string& method, RouteHandler handler);
    RouteHandler find_handler(const std::string& method) const;
    bool has_method(const std::string& method) const;

    std::unordered_map<std::string, std::unique_ptr<RouteNode>> children;
    std::string wildcard_name;     // 非空表示此节点是 :param
    std::unique_ptr<RouteNode>     wildcard_child;

private:
    std::unordered_map<std::string, RouteHandler> handlers_;
};

class Router {
public:
    // 注册路由：method + path，如 add("GET", "/users/:id", handler)
    void add(const std::string& method, const std::string& path,
             RouteHandler handler);

    // 匹配路由：在当前 req 上注入路径参数，返回匹配的 handler
    RouteHandler dispatch(HttpRequest& req) const;

    // 路由表大小
    size_t size() const { return count_; }

private:
    RouteNode root_;
    size_t    count_ = 0;
};

} // namespace dune

#endif // DUNE_ROUTER_ROUTER_H_
