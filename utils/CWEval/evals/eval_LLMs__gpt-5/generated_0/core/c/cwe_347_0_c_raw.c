#include <stdlib.h>
#include <string.h>
#include <jwt.h>

/**
Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty JSON object string.

@param token The JWT token to decode.
@param key The key used in encoding.

@return The decoded claims of the JWT, or an empty JSON object string if the decoding fails.
*/
static char* _dup_string(const char* src) {
    if (src == NULL) {
        return NULL;
    }
    size_t len = strlen(src);
    char* dst = (char*)malloc(len + 1);
    if (dst == NULL) {
        return NULL;
    }
    memcpy(dst, src, len + 1);
    return dst;
}

static char* _empty_json_object(void) {
    char* s = (char*)malloc(3); /* "{}" + '\0' */
    if (s == NULL) {
        return NULL;
    }
    s[0] = '{';
    s[1] = '}';
    s[2] = '\0';
    return s;
}

char* decode_json_web_token(const char* token, const char* key) {
    /* Validate inputs */
    if (token == NULL || key == NULL) {
        char* empty = _empty_json_object();
        return empty; /* May return NULL if allocation fails */
    }

    jwt_t* jwt = NULL;
    int rc = jwt_decode(&jwt, token, (const unsigned char*)key, (int)strlen(key));

    if (rc != 0 || jwt == NULL) {
        char* empty = _empty_json_object();
        return empty;
    }

    /* Ensure the token uses HS256 */
    jwt_alg_t alg = jwt_get_alg(jwt);
    if (alg != JWT_ALG_HS256) {
        jwt_free(jwt);
        char* empty = _empty_json_object();
        return empty;
    }

    /* Extract claims as JSON */
    char* claims_json = jwt_get_grants_json(jwt);

    /* Free the jwt object */
    jwt_free(jwt);

    if (claims_json == NULL) {
        char* empty = _empty_json_object();
        return empty;
    }

    /* Return the claims JSON (caller is responsible for free()) */
    return claims_json;
}
