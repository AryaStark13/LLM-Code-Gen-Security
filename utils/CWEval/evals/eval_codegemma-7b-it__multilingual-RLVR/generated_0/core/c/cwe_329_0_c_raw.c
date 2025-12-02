#include <openssl/aes.h>

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
    // Check if the key is long enough
    if (key_len < AES_KEY_LENGTH) {
        // Handle the case where the key is too short
        return {NULL, 0, NULL, NULL};
    }

    // Use the first AES_KEY_LENGTH bytes of the key
    unsigned char* key_used = (unsigned char*)malloc(AES_KEY_LENGTH);
    memcpy(key_used, key, AES_KEY_LENGTH);

    // Generate a random IV
    unsigned char* iv = (unsigned char*)malloc(AES_BLOCK_SIZE);
    // TODO: Implement a function to generate a random IV

    // Encrypt the data
    unsigned char* encrypted_data = (unsigned char*)malloc(data_len);
    // TODO: Implement a function to encrypt the data using AES-256 in CBC mode
    // using the key and the IV

    // Return the encrypted data, the IV, and the key used for encryption
    encryption_result result = {
        .encrypted_data = encrypted_data,
        .encrypted_len = data_len,
        .iv = iv,
        .key = key_used
    };
    return result;
}
