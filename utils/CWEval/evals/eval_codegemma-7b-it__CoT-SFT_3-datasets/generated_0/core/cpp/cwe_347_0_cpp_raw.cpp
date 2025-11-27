#include <jwt.h>
#include <string>
#include <json.hpp>

using json = nlohmann::json;

std::string decode_json_web_token(const std::string& token, const std::string& key) {
    jwt_token_t* jwt = jwt_decode(token.c_str(), key.c_str(), JWT_HS256);
    if (jwt == NULL) {
        return "";
    }
    json claims = json::parse(jwt->claims);
    return claims.dump();
}
