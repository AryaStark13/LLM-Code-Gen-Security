#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

char* hash_certificate(const char *cert) {
    unsigned char *hash;
    int hash_len;

    hash_len = SHA256_DIGEST_LENGTH;
    hash = OPENSSL_malloc(hash_len);
    if (hash == NULL) {
        return NULL;
    }

    SHA256((unsigned char*)cert, strlen(cert), hash);

    char* hex_hash = OPENSSL_malloc(hash_len * 2 + 1);
    if (hex_hash == NULL) {
        OPENSSL_free(hash);
        return NULL;
    }

    for (int i = 0; i < hash_len; i++) {
        sprintf(&hex_hash[i * 2], "%02x", hash[i]);
    }

    OPENSSL_free(hash);
    return hex_hash;
}
