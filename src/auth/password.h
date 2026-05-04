#ifndef DUNE_AUTH_PASSWORD_H_
#define DUNE_AUTH_PASSWORD_H_

#include <string>

namespace dune {
namespace password {

// 生成盐+哈希：返回 "salt_hex$hash_hex"
std::string hash(const std::string& password);

// 验证密码：password 明文 vs stored（"salt$hash" 格式）
bool verify(const std::string& password, const std::string& stored);

} // namespace password
} // namespace dune

#endif // DUNE_AUTH_PASSWORD_H_
