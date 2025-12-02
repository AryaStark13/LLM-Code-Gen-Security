#include <jwt-cpp/jwt.h>

std::string decode_json_web_token(const std::string& token, const std::string& key) {
    try {
        auto decoded = jwt::decode(token);
        auto header = decoded.get_header();
        auto payload = decoded.get_payload();
        // Convert payload to JSON string
        std::string json_payload = payload.dump();
        return json_payload;
    } catch (const std::exception& e) {
        // Handle decoding error
        return "{}";
    }
}
