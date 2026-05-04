#ifndef DUNE_MIDDLEWARE_MIDDLEWARE_H_
#define DUNE_MIDDLEWARE_MIDDLEWARE_H_

#include <functional>
#include <vector>
#include <memory>

namespace dune {

struct HttpRequest;
class HttpResponse;

// Next 函数：调用下一个中间件或最终 handler
using Next = std::function<void()>;

class Middleware {
public:
    virtual ~Middleware() = default;

    // 处理请求：
    //   1. 做前置处理
    //   2. 调用 next() 进入下一个中间件
    //   3. 做后置处理
    virtual void process(HttpRequest& req, HttpResponse& resp, Next next) = 0;
};

class MiddlewareChain {
public:
    // 添加中间件（越早添加越靠前执行）
    void add(std::unique_ptr<Middleware> mw);

    // 设置最终 handler
    void set_handler(std::function<void(HttpRequest&, HttpResponse&)> handler);

    // 启动中间件链
    void run(HttpRequest& req, HttpResponse& resp);

private:
    void execute(size_t index, HttpRequest& req, HttpResponse& resp);

    std::vector<std::unique_ptr<Middleware>> middlewares_;
    std::function<void(HttpRequest&, HttpResponse&)> handler_;
    size_t current_ = 0;
};

} // namespace dune

#endif // DUNE_MIDDLEWARE_MIDDLEWARE_H_
