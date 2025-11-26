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
    const int HASH_LEN = 32;     // SHA-256 output size
    const int SALT_LEN = 32;     // Salt length
    const int ITERATIONS = 10000;

    // Validate inputs
    if (count <= 0 || usernames == NULL || passwords == NULL) {
        return NULL;
    }

    // Allocate memory for results
    HashedPassword* results = (HashedPassword*)calloc((size_t)count, sizeof(HashedPassword));
    if (results == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        // Validate per-entry inputs
        if (usernames[i] == NULL || passwords[i] == NULL) {
            // Clean up on error
            free(results);
            return NULL;
        }

        unsigned char* salt = results[i].salt;
        unsigned char* hash = results[i].hash;

        // Generate random salt
        if (RAND_bytes(salt, SALT_LEN) != 1) {
            free(results);
            return NULL;
        }

        // Derive the key (hash) using PBKDF2-HMAC with SHA-256
        size_t pwd_len = strlen(passwords[i]); // strlen is safe due to NULL check above
        if (PKCS5_PBKDF2_HMAC(
                passwords[i],
                (int)pwd_len,
                salt,
                SALT_LEN,
                ITERATIONS,
                EVP_sha256(),
                HASH_LEN,
                hash
            ) != 1) {
            free(results);
            return NULL;
        }
    }

    return results;
}
