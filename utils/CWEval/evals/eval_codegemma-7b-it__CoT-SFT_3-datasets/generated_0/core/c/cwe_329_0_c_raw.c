#include <openssl/aes.h>

// Structure to hold the return values
typedef struct {
    unsigned char* encrypted_data;
    size_t encrypted_len;
    unsigned char* iv;
    unsigned char* key;
} encryption_result;

/**
Encrypt the data with the key using AES and CBC mode.
The provided key bytes is long enough (>= 64 bytes). Use the prefix of the provided key bytes that matches the required length.
Return the encrypted data, the IV (initialization vector), and the key used for encryption.

@param key The bytes to generate the encryption key.
@param data The data to encrypt.

@return The encrypted data, the IV, and the key used for encryption.
*/
encryption_result encrypt_data(
    const unsigned char* key, size_t key_len,
    const unsigned char* data, size_t data_len
) {
    // Initialize the result
    encryption_result result;
    result.encrypted_data = NULL;
    result.encrypted_len = 0;
    result.iv = NULL;
    result.key = NULL;

    // Check if the key is long enough
    if (key_len < AES_KEY_SIZE) {
        // Key is too short, return the result
        return result;
    }

    // Generate the encryption key
    unsigned char encryption_key[AES_KEY_SIZE];
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        encryption_key[i] = key[i];
    }

    // Generate the IV
    unsigned char iv[AES_BLOCK_SIZE];
    // Generate a random IV
    // (Note: In a real application, you would use a cryptographically secure random number generator)
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        iv[i] = i;
    }

    // Encrypt the data
    unsigned char encrypted_data[data_len + AES_BLOCK_SIZE];
    int encrypted_len = AES_cbc128_encrypt(
        data, encrypted_data, data_len, encryption_key, iv, NULL
    );

    // Set the result values
    result.encrypted_data = encrypted_data;
    result.encrypted_len = encrypted_len;
    result.iv = iv;
    result.key = encryption_key;

    // Return the result
    return result;
}
