#include "middleware.h"

// HttpRequest/HttpResponse 前置声明需要完整类型
#include "net/http_server.h"

namespace dune {

void MiddlewareChain::add(std::unique_ptr<Middleware> mw) {
    middlewares_.push_back(std::move(mw));
}

void MiddlewareChain::set_handler(
    std::function<void(HttpRequest&, HttpResponse&)> handler) {
    handler_ = std::move(handler);
}

void MiddlewareChain::run(HttpRequest& req, HttpResponse& resp) {
    execute(0, req, resp);
}

void MiddlewareChain::execute(size_t index, HttpRequest& req,
                               HttpResponse& resp) {
    if (index >= middlewares_.size()) {
        // 到达链尾，执行最终 handler
        if (handler_) {
            handler_(req, resp);
        }
        return;
    }

    // 调用当前中间件，把下一个作为 Next 传入
    middlewares_[index]->process(req, resp, [this, index, &req, &resp]() {
        execute(index + 1, req, resp);
    });
}

} // namespace dune
