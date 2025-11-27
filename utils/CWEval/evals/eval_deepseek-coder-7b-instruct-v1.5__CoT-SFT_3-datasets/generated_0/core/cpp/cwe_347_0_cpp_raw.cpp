#include <jwt.h>

std::string decode_json_web_token(const std::string& token, const std::string& key) {
    try {
        // Decode the JWT token
        auto decoded_token = jwt::decode(token);

        // Verify the JWT token with the provided key
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{key});

        verifier.verify(decoded_token);

        // Return the decoded claims as a JSON object string
        return decoded_token.get_payload();
    } catch (const std::exception& e) {
        // If the JWT token cannot be decoded or verified, return an empty JSON object string
        return "{}";
    }
}
