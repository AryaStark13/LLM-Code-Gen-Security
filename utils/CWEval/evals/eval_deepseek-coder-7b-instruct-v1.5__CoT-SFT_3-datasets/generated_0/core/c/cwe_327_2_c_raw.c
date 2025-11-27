#include <openssl/aes.h>
#include <openssl/rand.h>

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

    // Generate a random IV
    if (RAND_bytes(result.iv, AES_BLOCK_SIZE) <= 0) {
        // Handle error
    }

    // Use the provided key to generate the encryption key
    memcpy(result.key, key, key_len);

    // Encrypt the data
    AES_KEY aes_key;
    if (AES_set_encrypt_key(result.key, key_len * 8, &aes_key) < 0) {
        // Handle error
    }
    AES_cbc_encrypt(data, result.encrypted_data, data_len, &aes_key, result.iv, AES_ENCRYPT);

    result.encrypted_len = data_len;

    return result;
}
