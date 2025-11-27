#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <vector>

// Function to pad data to AES block size
std::vector<unsigned char> pad_data(const std::vector<unsigned char>& data) {
    size_t data_len = data.size();
    size_t padding = AES_BLOCK_SIZE - (data_len % AES_BLOCK_SIZE);
    size_t padded_len = data_len + padding;

    std::vector<unsigned char> padded_data(data.begin(), data.end());
    padded_data.resize(padded_len, (unsigned char)padding);

    return padded_data;
}

typedef struct {
    std::vector<unsigned char> encrypted_data;
    std::vector<unsigned char> iv;
    std::vector<unsigned char> key;
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
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& data
) {
    // Prepare the data for encryption
    std::vector<unsigned char> padded_data = pad_data(data);

    // Generate an initialization vector (IV)
    std::vector<unsigned char> iv(AES_BLOCK_SIZE, 0);
    RAND_bytes(iv.data(), iv.size());

    // Generate a key from the provided bytes
    std::vector<unsigned char> key_256(key.begin(), key.begin() + 32);

    // Encrypt the padded data using AES-256-CBC
    std::vector<unsigned char> encrypted_data(padded_data.size(), 0);
    AES_KEY aes_key;
    AES_set_encrypt_key(key_256.data(), 256, &aes_key);
    AES_cbc_encrypt(padded_data.data(), encrypted_data.data(), padded_data.size(), &aes_key, iv.data(), AES_ENCRYPT);

    // Return the encrypted data, IV, and key used for encryption
    encryption_result result;
    result.encrypted_data = encrypted_data;
    result.iv = iv;
    result.key = key_256;
    return result;
}
