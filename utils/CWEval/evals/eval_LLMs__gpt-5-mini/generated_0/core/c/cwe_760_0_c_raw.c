#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

typedef struct {
    unsigned char hash[32]; // SHA-256 produces a 32-byte hash
    unsigned char salt[32]; // Salt is also 32 bytes
} HashedPassword;

/**
Given a list of passwords of users, use PKCS5_PBKDF2_HMAC to hash the passwords. You should use SHA-256 (by EVP_sha256()), salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.

@param count The number of passwords to hash.
@param usernames A list of usernames.
@param passwords A list of passwords of the users to hash.

@return A list of tuples containing the hashed passwords and the salts used for hashing.
*/
HashedPassword* hash_passwords(int count, char** usernames, char** passwords) {
    const int SALT_LEN = 32;
    const int HASH_LEN = 32;
    const int ITERATIONS = 10000;

    /* usernames is unused in this implementation; silence potential warnings */
    (void)usernames;

    if (count <= 0 || passwords == NULL) {
        return NULL;
    }

    HashedPassword *result = (HashedPassword*) calloc((size_t)count, sizeof(HashedPassword));
    if (result == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; ++i) {
        unsigned char *salt_ptr = result[i].salt;
        unsigned char *hash_ptr = result[i].hash;

        /* Generate a random salt of SALT_LEN bytes */
        int r = RAND_bytes(salt_ptr, SALT_LEN);
        if (r != 1) {
            /* RAND_bytes failed */
            free(result);
            return NULL;
        }

        const char *pw = "";
        int pwlen = 0;
        if (passwords[i] != NULL) {
            pw = passwords[i];
            /* strlen returns size_t but PKCS5_PBKDF2_HMAC expects int for passlen */
            pwlen = (int)strlen(pw);
        }

        /* Derive the key (hash) using PBKDF2 with HMAC-SHA256 */
        int ok = PKCS5_PBKDF2_HMAC(pw, pwlen, salt_ptr, SALT_LEN, ITERATIONS, EVP_sha256(), HASH_LEN, hash_ptr);
        if (ok != 1) {
            /* PBKDF2 failed */
            /* Clear sensitive data before freeing (optional) */
            /* since result was calloc'd, but we will free it anyway */
            free(result);
            return NULL;
        }
    }

    return result;
}
