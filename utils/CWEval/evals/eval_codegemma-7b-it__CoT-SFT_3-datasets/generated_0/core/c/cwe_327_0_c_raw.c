#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function to hash the given certificate securely
char* hash_certificate(const char *cert) {
    // Initialize a SHA-256 context
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    // Update the context with the certificate data
    SHA256_Update(&sha256, cert, strlen(cert));

    // Compute the hash and store it in a buffer
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    // Convert the hash into a hexadecimal string
    char *hex = bin2hex(hash, SHA256_DIGEST_LENGTH);

    // Return the hexadecimal string
    return hex;
}
