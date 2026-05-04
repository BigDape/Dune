#ifndef DUNE_AUTH_AUTH_MW_H_
#define DUNE_AUTH_AUTH_MW_H_

#include <string>
#include "middleware/middleware.h"

namespace dune {

// 需要认证的接口会被此中间件拦截
// 从 Authorization: Bearer <token> 中提取 JWT 并验证
// 验证通过：将 user_id 注入 req.params["_user_id"]
// 验证失败：直接返回 401
class AuthMiddleware : public Middleware {
public:
    explicit AuthMiddleware(const std::string& jwt_secret)
        : secret_(jwt_secret) {}

    void process(HttpRequest& req, HttpResponse& resp, Next next) override;

    // 用于排除不需要认证的路径（如 /api/auth/login）
    AuthMiddleware& skip(const std::string& prefix);

private:
    std::string secret_;
    std::vector<std::string> skip_prefixes_;
};

} // namespace dune

#endif // DUNE_AUTH_AUTH_MW_H_
