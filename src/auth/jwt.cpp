#include "jwt.h"
#include "log/logger.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <cstring>
#include <sstream>

namespace dune {
namespace jwt {

// ------------ Base64Url 编解码 ------------
static const char BASE64_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64url_encode(const std::string& data) {
    std::string out;
    size_t len = data.size();
    for (size_t i = 0; i < len; i += 3) {
        unsigned int v = (unsigned char)data[i] << 16;
        if (i + 1 < len) v |= (unsigned char)data[i + 1] << 8;
        if (i + 2 < len) v |= (unsigned char)data[i + 2];

        out += BASE64_TABLE[(v >> 18) & 0x3F];
        out += BASE64_TABLE[(v >> 12) & 0x3F];
        out += (i + 1 < len) ? BASE64_TABLE[(v >> 6) & 0x3F] : '=';
        out += (i + 2 < len) ? BASE64_TABLE[v & 0x3F] : '=';
    }
    // Replace for URL-safe: + → -, / → _, remove padding =
    for (auto& c : out) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    while (!out.empty() && out.back() == '=') out.pop_back();
    return out;
}

static int base64_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '-' || c == '+') return 62;
    if (c == '_' || c == '/') return 63;
    return 0;
}

static std::string base64url_decode(const std::string& input) {
    std::string data = input;
    // Restore padding
    while (data.size() % 4 != 0) data += '=';
    // Restore chars
    for (auto& c : data) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }

    std::string out;
    for (size_t i = 0; i < data.size(); i += 4) {
        int v = (base64_value(data[i]) << 18)
              + (base64_value(data[i + 1]) << 12)
              + (base64_value(data[i + 2]) << 6)
              +  base64_value(data[i + 3]);
        out += (char)((v >> 16) & 0xFF);
        if (data[i + 2] != '=') out += (char)((v >> 8) & 0xFF);
        if (data[i + 3] != '=') out += (char)(v & 0xFF);
    }
    return out;
}

// ------------ HMAC-SHA256 ------------
static std::string hmac_sha256(const std::string& data, const std::string& key) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;

    HMAC(EVP_sha256(),
         key.c_str(), key.size(),
         (const unsigned char*)data.c_str(), data.size(),
         result, &len);

    return std::string((char*)result, len);
}

// ------------ Public API ------------
std::string encode(const std::string& payload, const std::string& secret) {
    std::string header = R"({"alg":"HS256","typ":"JWT"})";
    std::string h = base64url_encode(header);
    std::string p = base64url_encode(payload);
    std::string sig = hmac_sha256(h + "." + p, secret);
    std::string s = base64url_encode(sig);
    return h + "." + p + "." + s;
}

std::string decode(const std::string& token, const std::string& secret) {
    // Split by '.'
    auto p1 = token.find('.');
    auto p2 = token.find('.', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos) {
        return "";
    }

    std::string h   = token.substr(0, p1);
    std::string p   = token.substr(p1 + 1, p2 - p1 - 1);
    std::string sig = token.substr(p2 + 1);

    // Verify signature
    std::string expected_sig = base64url_encode(
        hmac_sha256(h + "." + p, secret));
    if (sig != expected_sig) {
        LOG_DEBUG("JWT signature mismatch");
        return "";
    }

    return base64url_decode(p);
}

} // namespace jwt
} // namespace dune
