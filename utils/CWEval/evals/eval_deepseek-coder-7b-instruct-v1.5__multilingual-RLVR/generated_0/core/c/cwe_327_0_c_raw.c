#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>

char* hash_certificate(const char *cert) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, cert, strlen(cert));
    SHA256_Final(hash, &sha256);

    char* output = malloc(2 * SHA256_DIGEST_LENGTH + 1);
    if (output == NULL) {
        return NULL;
    }

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&output[i*2], "%02x", hash[i]);
    }

    return output;
}
