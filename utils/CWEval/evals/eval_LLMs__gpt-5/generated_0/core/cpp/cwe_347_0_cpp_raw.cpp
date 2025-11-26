#include <jwt.h>
#include <string>
#include <memory>
#include <cstdlib>

/**
Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty JSON object string.

@param token The JWT token to decode.
@param key The key used in encoding.

@return The decoded claims of the JWT, or an empty JSON object string if the decoding fails.
*/
std::string decode_json_web_token(const std::string& token, const std::string& key) {
    // Return empty JSON object on invalid inputs
    if (token.empty() || key.empty()) {
        return "{}";
    }

    // Decode the JWT using libjwt
    jwt_t* raw_jwt = nullptr;
    const unsigned char* ukey = reinterpret_cast<const unsigned char*>(key.data());
    int key_len = static_cast<int>(key.size());

    int decode_ret = jwt_decode(&raw_jwt, token.c_str(), ukey, key_len);
    if (decode_ret != 0 || raw_jwt == nullptr) {
        return "{}";
    }

    // RAII wrapper to free jwt_t
    struct JwtDeleter {
        void operator()(jwt_t* p) const {
            if (p) jwt_free(p);
        }
    };
    std::unique_ptr<jwt_t, JwtDeleter> jwt_ptr(raw_jwt);

    // Ensure the algorithm is HS256
    // If the library supports jwt_get_alg, verify it; otherwise, proceed to verification.
    // Many libjwt versions provide jwt_get_alg and JWT_ALG_HS256.
    #ifdef JWT_ALG_HS256
    jwt_alg_t alg = jwt_get_alg(jwt_ptr.get());
    if (alg != JWT_ALG_HS256) {
        return "{}";
    }
    #endif

    // Verify signature using HS256 with the provided key
    int verify_ret = jwt_verify(jwt_ptr.get(), ukey, key_len);
    if (verify_ret != 0) {
        return "{}";
    }

    // Extract claims as JSON
    char* grants_json = nullptr;
    int grants_ret = jwt_get_grants_json(jwt_ptr.get(), &grants_json);
    if (grants_ret != 0 || grants_json == nullptr) {
        return "{}";
    }

    // RAII wrapper to free char* allocated by libjwt
    struct CStrDeleter {
        void operator()(char* p) const {
            if (p) free(p);
        }
    };
    std::unique_ptr<char, CStrDeleter> grants_ptr(grants_json);

    // Convert to std::string and return
    return std::string(grants_ptr.get());
}
