#include <jwt.h>

/**
Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty JSON object string.

@param token The JWT token to decode.
@param key The key used in encoding.

@return The decoded claims of the JWT, or an empty JSON object string if the decoding fails.
*/
char* decode_json_web_token(const char* token, const char* key) {
    // Decode the JWT
    json_t* claims = jwt_decode(token, key, NULL);
    if (!claims) {
        return "";
    }

    // Verify the signature
    if (!jwt_verify(token, key, NULL)) {
        json_decref(claims);
        return "";
    }

    // Convert the claims to a JSON string
    char* claims_string = json_dumps(claims, JSON_ENCODE_ANY);
    json_decref(claims);

    return claims_string;
}
