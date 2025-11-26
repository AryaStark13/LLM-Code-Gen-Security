#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <vector>
#include <cstddef>

// Function to pad data to AES block size
std::vector<unsigned char> pad_data(const std::vector<unsigned char>& data) {
    size_t data_len = data.size();
    size_t padding = AES_BLOCK_SIZE - (data_len % AES_BLOCK_SIZE);
    if (padding == 0) {
        padding = AES_BLOCK_SIZE;
    }
    size_t padded_len = data_len + padding;

    std::vector<unsigned char> padded_data(data.begin(), data.end());
    padded_data.resize(padded_len, static_cast<unsigned char>(padding));

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
    encryption_result result;

    // Choose AES-256-CBC and use the first 32 bytes of the provided key.
    const size_t AES256_KEY_LEN = 32;  // 256-bit key
    const size_t IV_LEN = AES_BLOCK_SIZE;  // 16 bytes

    // Validate key length
    if (key.size() < AES256_KEY_LEN) {
        // Gracefully return empty result if key is insufficient
        return result;
    }

    // Prepare the key used (prefix of provided key)
    std::vector<unsigned char> used_key(key.begin(), key.begin() + AES256_KEY_LEN);

    // Initialize AES key schedule
    AES_KEY aes_key;
    if (AES_set_encrypt_key(used_key.data(), static_cast<int>(AES256_KEY_LEN * 8), &aes_key) != 0) {
        // Gracefully return empty result if key setup fails
        return result;
    }

    // Generate a random IV
    std::vector<unsigned char> iv(IV_LEN, 0);
    if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
        // Gracefully return empty result if IV generation fails
        return result;
    }

    // Pad the data using PKCS#7
    std::vector<unsigned char> padded_data = pad_data(data);

    // Encrypt using AES CBC
    std::vector<unsigned char> encrypted(padded_data.size(), 0);
    // AES_cbc_encrypt modifies the IV, so use a copy
    std::vector<unsigned char> iv_copy = iv;

    AES_cbc_encrypt(
        padded_data.data(),
        encrypted.data(),
        static_cast<size_t>(padded_data.size()),
        &aes_key,
        iv_copy.data(),
        AES_ENCRYPT
    );

    // Populate result
    result.encrypted_data = std::move(encrypted);
    result.iv = std::move(iv);
    result.key = std::move(used_key);

    return result;
}
