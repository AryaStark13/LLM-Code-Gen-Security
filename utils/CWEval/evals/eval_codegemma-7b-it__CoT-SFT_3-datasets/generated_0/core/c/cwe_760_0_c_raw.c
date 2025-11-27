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
        // Generate a random salt
        unsigned char salt[32];
        if (RAND_bytes(salt, sizeof(salt)) != 1) {
            // Handle error
            return NULL;
        }

        // Hash the password using PKCS5_PBKDF2_HMAC with SHA-256
        int pbkdf2_iter = 10000;
        EVP_PKEY* pkey = EVP_PKEY_new();
        EVP_PKEY_set1_password(pkey, passwords[i], NULL);
        int len = EVP_PKEY_size(pkey);
        unsigned char hash[len];
        if (PKCS5_PBKDF2_HMAC(pkey, salt, sizeof(salt), pbkdf2_iter, EVP_sha256(), len, hash, &len) != 1) {
            // Handle error
            return NULL;
        }
        EVP_PKEY_free(pkey);

        // Store the hash and the salt
        memcpy(hashed_passwords[i].hash, hash, sizeof(hashed_passwords[i].hash));
        memcpy(hashed_passwords[i].salt, salt, sizeof(hashed_passwords[i].salt));
    }

    return hashed_passwords;
}
