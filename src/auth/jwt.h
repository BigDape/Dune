#ifndef DUNE_AUTH_JWT_H_
#define DUNE_AUTH_JWT_H_

#include <string>
#include <cstdint>

namespace dune {
namespace jwt {

// 使用 HS256 生成 JWT token
// payload: JSON 字符串，如 {"sub":"123","iat":1234567890}
std::string encode(const std::string& payload, const std::string& secret);

// 验证 JWT token，成功返回 payload JSON，失败返回空串
std::string decode(const std::string& token, const std::string& secret);

} // namespace jwt
} // namespace dune

#endif // DUNE_AUTH_JWT_H_
