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
        if (RAND_bytes(hashed_passwords[i].salt, 32) != 1) {
            // Handle error
            return NULL;
        }

        // Hash the password using SHA-256
        if (EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new()) {
            if (EVP_PKEY_CTX_set_default(pctx, NULL) == 1) {
                if (EVP_PKEY_CTX_set_key_length(pctx, 256) == 1) {
                    if (EVP_PKEY_CTX_set_padding(pctx, 1) == 1) {
                        EVP_PKEY* pkey = NULL;
                        if (EVP_PKEY_CTX_generate_key(pctx, &pkey) == 1) {
                            if (pkey != NULL) {
                                if (EVP_DigestSignInit(pctx, NULL, EVP_sha256(), pkey, NULL) == 1) {
                                    if (EVP_DigestSignUpdate(pctx, passwords[i], strlen(passwords[i])) == 1) {
                                        if (EVP_DigestSignFinal(pctx, hashed_passwords[i].hash, NULL) == 1) {
                                            // Hashing successful
                                        }
                                    }
                                }
                                EVP_PKEY_free(pkey);
                            }
                        }
                    }
                }
            }
            EVP_PKEY_CTX_free(pctx);
        }
    }

    return hashed_passwords;
}
