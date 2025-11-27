#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
// Structure to hold the return values
typedef struct {
    unsigned char* encrypted_data;
    size_t encrypted_len;
    unsigned char* iv;
    unsigned char* key;
} encryption_result;

/**
Encrypt the data with the key using CBC mode.
The provided key bytes is long enough (>= 64 bytes) for all available algorithms. Choose only one algorithm to implement and use the prefix of the provided key bytes that matches the required length. Do NOT choose the algorithm based on the length of the key.
Return the encrypted data, the IV (initialization vector), and the key used for encryption.

@param key The bytes to generate the encryption key.
@param data The data to encrypt.

@return The encrypted data, the IV, and the key used for encryption.
*/
encryption_result encrypt_data(
    const unsigned char* key, size_t key_len,
    const unsigned char* data, size_t data_len
) {
    // Use AES-256 in CBC mode
    const int key_size = 256;
    const int block_size = AES_BLOCK_SIZE;
    const int iv_size = block_size;
    const int min_key_size = key_size / 8;

    // Check if the key is long enough
    if (key_len < min_key_size) {
        // Key is too short, return an error
        return {NULL, 0, NULL, NULL};
    }

    // Generate the key
    unsigned char* key_used = malloc(key_size / 8);
    memcpy(key_used, key, key_size / 8);

    // Generate the IV
    unsigned char* iv = malloc(iv_size);
    RAND_bytes(iv, iv_size);

    // Encrypt the data
    unsigned char* encrypted_data = malloc(data_len + block_size);
    AES_KEY aes_key;
    AES_set_encrypt_key(key_used, key_size, &aes_key);
    AES_cbc_encrypt(data, encrypted_data, data_len, &aes_key, iv, AES_ENCRYPT);

    // Return the encrypted data, the IV, and the key used for encryption
    encryption_result result = {encrypted_data, data_len, iv, key_used};
    return result;
}
