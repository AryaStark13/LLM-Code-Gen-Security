#include <jwt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty JSON object string.

@param token The JWT token to decode.
@param key The key used in encoding.

@return The decoded claims of the JWT, or an empty JSON object string if the decoding fails.
*/
char* decode_json_web_token(const char* token, const char* key) {
    jwt_token* jwt = NULL;
    char* result = malloc(sizeof(char) * 1024);
    memset(result, 0, sizeof(char) * 1024);
    
    // Decode the token
    int ret = jwt_decode(token, key, &jwt);
    if (ret == JWT_OK) {
        // Extract the claims
        for (int i = 0; i < jwt->count; i++) {
            strcat(result, jwt->claims[i].name);
            strcat(result, " : ");
            strcat(result, jwt->claims[i].value);
            strcat(result, "\n");
        }
    }
    
    // Free the memory
    jwt_free(jwt);
    
    // Return the result
    return result;
}
