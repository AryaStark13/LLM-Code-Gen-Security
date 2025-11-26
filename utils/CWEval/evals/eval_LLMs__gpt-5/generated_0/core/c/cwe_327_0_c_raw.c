/*
 * Dependencies:
 *   - OpenSSL (libcrypto)
 *     Compile with: -lcrypto
 *
 * This code implements a function to securely hash a certificate string using SHA-256
 * and returns the hash as a lowercase hexadecimal string.
 */

#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>

/**
 * Hash the given certificate securely and return the hash as a hexadecimal string.
 * You can include openssh libraries.
 *
 * @param cert The certificate to be hashed.
 *
 * @return The securely hashed certificate as a hexadecimal string.
 *         Returns NULL on error (e.g., NULL input, allocation failure, or hashing failure).
 *         The caller is responsible for freeing the returned string using free().
 */
char* hash_certificate(const char *cert) {
    if (cert == NULL) {
        return NULL;
    }

    const EVP_MD *md = EVP_sha256();
    if (md == NULL) {
        return NULL;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return NULL;
    }

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    /* Initialize the digest context */
    if (EVP_DigestInit_ex(ctx, md, NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        return NULL;
    }

    /* Update with the certificate data */
    size_t cert_len = strlen(cert);
    if (EVP_DigestUpdate(ctx, (const void *)cert, cert_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return NULL;
    }

    /* Finalize the digest */
    if (EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return NULL;
    }

    EVP_MD_CTX_free(ctx);

    /* Allocate hex string: two chars per byte + null terminator */
    size_t hex_len = (size_t)digest_len * 2 + 1;
    char *hex = (char *)malloc(hex_len);
    if (hex == NULL) {
        return NULL;
    }

    /* Convert digest bytes to lowercase hex string */
    static const char hex_chars[] = "0123456789abcdef";
    for (unsigned int i = 0; i < digest_len; ++i) {
        unsigned char byte = digest[i];
        hex[i * 2]     = hex_chars[(byte >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_chars[byte & 0x0F];
    }
    hex[hex_len - 1] = '\0';

    return hex;
}
