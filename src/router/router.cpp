#include "router.h"
#include "net/http_server.h"
#include <sstream>

namespace dune {

// ===================== RouteNode =====================

void RouteNode::add_handler(const std::string& method, RouteHandler handler) {
    handlers_[method] = std::move(handler);
}

RouteHandler RouteNode::find_handler(const std::string& method) const {
    auto it = handlers_.find(method);
    return (it != handlers_.end()) ? it->second : nullptr;
}

bool RouteNode::has_method(const std::string& method) const {
    return handlers_.find(method) != handlers_.end();
}

// ===================== Router =====================

void Router::add(const std::string& method, const std::string& path,
                 RouteHandler handler) {
    RouteNode* node = &root_;

    std::istringstream ss(path);
    std::string segment;

    // 跳过第一个空段（path 以 / 开头）
    while (std::getline(ss, segment, '/')) {
        if (segment.empty()) continue;

        if (segment[0] == ':') {
            // 动态参数段
            std::string param_name = segment.substr(1);
            if (!node->wildcard_child) {
                node->wildcard_child = std::make_unique<RouteNode>();
                node->wildcard_name = param_name;
            }
            node = node->wildcard_child.get();
        } else {
            // 静态段
            auto& child = node->children[segment];
            if (!child) {
                child = std::make_unique<RouteNode>();
            }
            node = child.get();
        }
    }

    node->add_handler(method, std::move(handler));
    count_++;
}

RouteHandler Router::dispatch(HttpRequest& req) const {
    const RouteNode* node = &root_;

    std::istringstream ss(req.path);
    std::string segment;

    while (std::getline(ss, segment, '/')) {
        if (segment.empty()) continue;

        // 优先匹配静态
        auto it = node->children.find(segment);
        if (it != node->children.end()) {
            node = it->second.get();
        } else if (node->wildcard_child) {
            req.params[node->wildcard_name] = segment;
            node = node->wildcard_child.get();
        } else {
            return nullptr;
        }
    }

    return node->find_handler(req.method);
}

} // namespace dune
