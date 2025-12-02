#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h>

char* hash_certificate(const char *cert) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    char *hex_hash = NULL;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, cert, strlen(cert));
    SHA256_Final(hash, &sha256);

    hex_hash = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    if (hex_hash == NULL) {
        return NULL;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hex_hash[i * 2], "%02x", hash[i]);
    }
    hex_hash[SHA256_DIGEST_LENGTH * 2] = '\0';

    return hex_hash;
}
