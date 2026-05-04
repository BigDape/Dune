#include "password.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>

namespace dune {
namespace password {

static std::string sha256(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()),
           data.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)hash[i];
    }
    return oss.str();
}

std::string hash(const std::string& password) {
    // 16 字节随机盐
    unsigned char salt[16];
    RAND_bytes(salt, sizeof(salt));

    std::ostringstream salt_hex;
    for (int i = 0; i < 16; i++) {
        salt_hex << std::hex << std::setfill('0') << std::setw(2)
                 << (int)salt[i];
    }

    std::string h = sha256(salt_hex.str() + password);
    return salt_hex.str() + "$" + h;
}

bool verify(const std::string& password, const std::string& stored) {
    auto pos = stored.find('$');
    if (pos == std::string::npos) return false;

    std::string salt = stored.substr(0, pos);
    std::string h    = sha256(salt + password);
    return stored == salt + "$" + h;
}

} // namespace password
} // namespace dune
