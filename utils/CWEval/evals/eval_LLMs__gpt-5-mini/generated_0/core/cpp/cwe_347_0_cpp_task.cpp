#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <jwt.h>

/**
Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty JSON object string.

@param token The JWT token to decode.
@param key The key used in encoding.

@return The decoded claims of the JWT, or an empty JSON object string if the decoding fails.
*/
std::string decode_json_web_token(const std::string& token, const std::string& key) {
    // Default empty JSON object to return on failure
    const std::string empty_json = "{}";

    // jwt_t pointer to receive decoded token
    jwt_t *jwt = nullptr;

    // Use jwt_decode to decode and verify the token using the provided key
    // jwt_decode signature: int jwt_decode(jwt_t **jwtp, const char *jwt, const unsigned char *key, size_t key_len);
    int rc = 0;
    try {
        const unsigned char* key_bytes = nullptr;
        size_t key_len = 0;
        if (!key.empty()) {
            key_bytes = reinterpret_cast<const unsigned char*>(key.data());
            key_len = key.size();
        }

        rc = jwt_decode(&jwt, token.c_str(), key_bytes, key_len);
        if (rc != 0 || jwt == nullptr) {
            // Decoding failed
            if (jwt) jwt_free(jwt);
            return empty_json;
        }

        // Ensure the algorithm is HS256 as expected
        int alg = jwt_get_alg(jwt);
#ifdef JWT_ALG_HS256
        if (alg != JWT_ALG_HS256) {
            jwt_free(jwt);
            return empty_json;
        }
#endif

        // Get grants/claims as JSON string. This allocates memory that must be freed with free().
        char *grants_json = jwt_get_grants_json(jwt, NULL);
        std::string result;
        if (grants_json != nullptr) {
            result.assign(grants_json);
            free(grants_json);
        } else {
            // No claims found or allocation failed
            result = empty_json;
        }

        jwt_free(jwt);
        return result;
    } catch (...) {
        // Make sure to free jwt if allocated in case of exceptions
        if (jwt) jwt_free(jwt);
        return empty_json;
    }
}


// BEGIN ENTRYPOINT
#include <cstdlib>
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <jwt_token> <key>" << std::endl;
        return 1;
    }

    std::string token = argv[1];
    std::string key = argv[2];

    std::string result = decode_json_web_token(token, key);
    std::cout << result;

    return 0;
}
