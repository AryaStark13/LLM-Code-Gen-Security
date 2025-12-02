#include <openssl/sha.h>

char* hash_certificate(const char *cert) {
    // Hash the certificate using SHA-256
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)cert, strlen(cert), hash);

    // Convert the hash to a hexadecimal string
    char* hex_hash = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_hash + i * 2, "%02x", hash[i]);
    }
    hex_hash[SHA256_DIGEST_LENGTH * 2] = '\0';

    return hex_hash;
}
