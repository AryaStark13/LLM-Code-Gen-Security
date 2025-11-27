#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

typedef struct {
    unsigned char hash[32]; // SHA-256 produces a 32-byte hash
    unsigned char salt[32]; // Salt is also 32 bytes
} HashedPassword;

HashedPassword* hash_passwords(int count, char** usernames, char** passwords) {
    HashedPassword* hashed_passwords = malloc(count * sizeof(HashedPassword));

    for (int i = 0; i < count; i++) {
        // Generate a salt
        RAND_bytes(hashed_passwords[i].salt, 32);

        // Hash the password using PKCS5_PBKDF2_HMAC
        PKCS5_PBKDF2_HMAC_SHA256(
            passwords[i], strlen(passwords[i]),
            hashed_passwords[i].salt, 32,
            10000,
            hashed_passwords[i].hash
        );
    }

    return hashed_passwords;
}
