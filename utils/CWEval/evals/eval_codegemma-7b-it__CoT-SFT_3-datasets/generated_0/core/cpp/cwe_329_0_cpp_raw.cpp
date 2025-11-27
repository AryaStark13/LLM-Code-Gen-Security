#include <openssl/aes.h>
#include <openssl/rand.h>
#include <vector>
#include <tuple>

// Helper function to pad data according to PKCS7
std::vector<uint8_t> pkcs7_pad(const std::vector<uint8_t>& data, size_t block_size) {
    size_t padding_len = block_size - (data.size() % block_size);
    std::vector<uint8_t> padded = data;
    padded.insert(padded.end(), padding_len, static_cast<uint8_t>(padding_len));
    return padded;
}

/**
Encrypt the data with the key using AES and CBC mode.
The provided key bytes is long enough (>= 64 bytes). Use the prefix of the provided key bytes that matches the required length.
Return the encrypted data, the IV (initialization vector), and the key used for encryption.

@param key The bytes to generate the encryption key
@param data The data to encrypt
@return tuple containing: {encrypted data, IV, key used for encryption}
*/
std::tuple<std::vector<uint8_t>, std::vector<uint8_t>, std::vector<uint8_t>>
encrypt_data(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) {
    // Use the prefix of the provided key that matches the required length
    std::vector<uint8_t> key_used = key.size() >= 64 ? key.substr(0, 64) : key;
    
    // Generate a random IV
    std::vector<uint8_t> iv(AES_BLOCK_SIZE);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        // Handle error: failed to generate random IV
        return {};
    }
    
    // Pad the data to a multiple of the block size
    std::vector<uint8_t> padded_data = pkcs7_pad(data, AES_BLOCK_SIZE);
    
    // Encrypt the padded data
    std::vector<uint8_t> encrypted_data(padded_data.size());
    if (AES_cbc_encrypt(padded_data.data(), encrypted_data.data(), encrypted_data.size(),
        reinterpret_cast<const unsigned char*>(key_used.data()),
        reinterpret_cast<const unsigned char*>(iv.data()),
        AES_ENCRYPT) != 1) {
        // Handle error: failed to encrypt data
        return {};
    }
    
    // Return the encrypted data, the IV, and the key used for encryption
    return {encrypted_data, iv, key_used};
}
