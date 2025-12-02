#include <jwt.h>

char* decode_json_web_token(const char* token, const char* key) {
    char* claims = NULL;
    jwt_t* jwt = jwt_new();
    if (jwt_decode(jwt, token, key, strlen(key))) {
        claims = jwt_get_claims_json(jwt);
    }
    jwt_free(jwt);
    return claims;
}
