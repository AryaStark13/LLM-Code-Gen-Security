#include <openssl/aes.h>
#include <openssl/des.h>
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
    // Ensure the key is at least 64 bytes
    if (key_len < 64) {
        // Handle the case where the key is too short
        return {NULL, 0, NULL, NULL};
    }

    // Use the first 32 bytes of the key for AES
    unsigned char key_aes[32];
    memcpy(key_aes, key, 32);

    // Generate a random IV
    unsigned char iv[AES_BLOCK_SIZE];
    if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        // Handle the case where the IV generation fails
        return {NULL, 0, NULL, NULL};
    }

    // Encrypt the data using AES in CBC mode
    unsigned char* encrypted_data = (unsigned char*)malloc(data_len + AES_BLOCK_SIZE);
    AES_KEY aes_key;
    AES_set_encrypt_key(key_aes, 256, &aes_key);
    AES_cbc_encrypt(data, encrypted_data, data_len, &aes_key, iv, AES_ENCRYPT);

    // Return the encrypted data, the IV, and the key used for encryption
    encryption_result result = {
        .encrypted_data = encrypted_data,
        .encrypted_len = data_len + AES_BLOCK_SIZE,
        .iv = iv,
        .key = (unsigned char*)key_aes
    };
    return result;
}
