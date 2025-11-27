#include <openssl/aes.h>
#include <openssl/rand.h>
#include <stdlib.h>

// Structure to hold the return values
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
    result.encrypted_data = malloc(data_len);
    result.iv = malloc(AES_BLOCK_SIZE);
    result.key = malloc(key_len);

    // Generate the key from the provided key bytes
    memcpy(result.key, key, key_len);

    // Generate a random IV
    if (RAND_bytes(result.iv, AES_BLOCK_SIZE) <= 0) {
        // Handle error
        free(result.encrypted_data);
        free(result.iv);
        free(result.key);
        return result;
    }

    // Encrypt the data
    AES_KEY aes_key;
    if (AES_set_encrypt_key(result.key, key_len * 8, &aes_key) < 0) {
        // Handle error
        free(result.encrypted_data);
        free(result.iv);
        free(result.key);
        return result;
    }

    unsigned char* encrypted_data = result.encrypted_data;
    size_t encrypted_len = data_len;
    if (AES_cbc_encrypt(data, encrypted_data, data_len, &aes_key, result.iv, AES_ENCRYPT) < 0) {
        // Handle error
        free(result.encrypted_data);
        free(result.iv);
        free(result.key);
        return result;
    }

    result.encrypted_len = encrypted_len;
    return result;
}
