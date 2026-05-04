#ifndef DUNE_MIDDLEWARE_BUILTIN_MW_H_
#define DUNE_MIDDLEWARE_BUILTIN_MW_H_

#include "middleware.h"

namespace dune {

// 请求日志中间件
class LoggerMiddleware : public Middleware {
public:
    void process(HttpRequest& req, HttpResponse& resp, Next next) override;
};

// CORS 跨域中间件
class CorsMiddleware : public Middleware {
public:
    void process(HttpRequest& req, HttpResponse& resp, Next next) override;
};

// 异常恢复中间件
class RecoveryMiddleware : public Middleware {
public:
    void process(HttpRequest& req, HttpResponse& resp, Next next) override;
};

} // namespace dune

#endif // DUNE_MIDDLEWARE_BUILTIN_MW_H_
