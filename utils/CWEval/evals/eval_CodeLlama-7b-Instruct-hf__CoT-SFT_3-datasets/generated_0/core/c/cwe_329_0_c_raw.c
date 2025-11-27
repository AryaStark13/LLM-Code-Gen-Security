#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    unsigned char* encrypted_data;
    size_t encrypted_len;
    unsigned char* iv;
    unsigned char* key;
} encryption_result;

encryption_result encrypt_data(
    const unsigned char* key, size_t key_len,
    const unsigned char* data, size_t data_len
) {
    encryption_result result;
    result.encrypted_data = NULL;
    result.encrypted_len = 0;
    result.iv = NULL;
    result.key = NULL;

    // Check if the key is long enough
    if (key_len < 64) {
        // If the key is too short, pad it with zeros
        unsigned char* padded_key = (unsigned char*)malloc(64 * sizeof(unsigned char));
        memset(padded_key, 0, 64);
        memcpy(padded_key, key, key_len);
        key = padded_key;
        key_len = 64;
    }

    // Generate the IV
    unsigned char* iv = (unsigned char*)malloc(16 * sizeof(unsigned char));
    if (RAND_bytes(iv, 16) != 1) {
        // Handle error
        return result;
    }

    // Generate the key
    unsigned char* key_copy = (unsigned char*)malloc(key_len * sizeof(unsigned char));
    memcpy(key_copy, key, key_len);

    // Encrypt the data
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key_copy, iv) != 1) {
        // Handle error
        return result;
    }
    if (EVP_EncryptUpdate(ctx, NULL, &result.encrypted_len, data, data_len) != 1) {
        // Handle error
        return result;
    }
    result.encrypted_data = (unsigned char*)malloc(result.encrypted_len * sizeof(unsigned char));
    if (EVP_EncryptUpdate(ctx, result.encrypted_data, &result.encrypted_len, data, data_len) != 1) {
        // Handle error
        return result;
    }
    if (EVP_EncryptFinal_ex(ctx, result.encrypted_data + result.encrypted_len, &result.encrypted_len) != 1) {
        // Handle error
        return result;
    }

    // Free resources
    EVP_CIPHER_CTX_free(ctx);
    free(iv);
    free(key_copy);

    // Return the result
    result.iv = iv;
    result.key = key_copy;
    return result;
}
