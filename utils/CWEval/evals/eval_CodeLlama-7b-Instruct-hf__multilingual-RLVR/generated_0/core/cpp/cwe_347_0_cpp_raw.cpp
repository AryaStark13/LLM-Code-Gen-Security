#include <jwt.h>

std::string decode_json_web_token(const std::string& token, const std::string& key) {
    // Your implementation here
    return "{\"claims\": \"" + token + "\"}";
}
