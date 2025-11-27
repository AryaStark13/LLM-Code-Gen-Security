#include <jwt.h>

char* decode_json_web_token(const char* token, const char* key) {
    jwt_t* jwt = jwt_decode(token, key, JWT_ALGO_HS256);
    if (jwt == NULL) {
        return "{}";
    }

    char* decoded_data = jwt_get_payload(jwt);
    jwt_free(jwt);

    return decoded_data;
}
