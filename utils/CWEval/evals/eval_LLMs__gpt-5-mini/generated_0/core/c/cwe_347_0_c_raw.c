#include <stdlib.h>
#include <string.h>
#include <jwt.h>

/* Helper: safe strdup that falls back to a static literal on allocation failure */
static char* safe_strdup_or_static(const char *s) {
    if (!s) {
        static char static_empty[] = "{}";
        return static_empty;
    }
    size_t len = strlen(s);
    char *copy = (char*)malloc(len + 1);
    if (!copy) {
        /* Allocation failed; return pointer to static literal (not freeable) */
        static char static_empty[] = "{}";
        if (strcmp(s, "{}") == 0) return static_empty;
        /* If s is not "{}", still return static empty object to satisfy contract */
        return static_empty;
    }
    memcpy(copy, s, len + 1);
    return copy;
}

/**
Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty JSON object string.

@param token The JWT token to decode.
@param key The key used in encoding.

@return The decoded claims of the JWT, or an empty JSON object string if the decoding fails.
*/
char* decode_json_web_token(const char* token, const char* key) {
    /* Validate inputs */
    if (!token || !key) {
        return safe_strdup_or_static("{}");
    }

    jwt_t *jwt = NULL;
    /* Decode using libjwt; pass key and its length. jwt_decode returns 0 on success. */
    int ret = jwt_decode(&jwt, token, (const unsigned char*)key, strlen(key));
    if (ret != 0 || jwt == NULL) {
        if (jwt) jwt_free(jwt);
        return safe_strdup_or_static("{}");
    }

    /* Get the claims as JSON. According to libjwt API, this returns a malloc'd string. */
    char *claims_json = jwt_get_grants_json(jwt, 0);
    /* Free the jwt object regardless */
    jwt_free(jwt);

    if (!claims_json) {
        return safe_strdup_or_static("{}");
    }

    /* Copy the claims JSON into caller-owned memory */
    char *result = safe_strdup_or_static(claims_json);
    /* Free the library-allocated claims string */
    free(claims_json);

    if (!result) {
        /* If safe_strdup_or_static returned static, ensure we return at least "{}" */
        return safe_strdup_or_static("{}");
    }
    return result;
}
