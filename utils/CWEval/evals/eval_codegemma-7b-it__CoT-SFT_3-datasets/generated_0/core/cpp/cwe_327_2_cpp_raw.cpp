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
    // Use AES-128 for illustration
    // The key and data are padded to the appropriate size
    // The key is also used as the IV
    // The IV is generated using a secure random number generator
    // The encrypted data is returned along with the key and IV
    
    // Pad the key to the AES block size
    std::vector<unsigned char> padded_key = pad_data(key);
    
    // Pad the data to the AES block size
    std::vector<unsigned char> padded_data = pad_data(data);
    
    // Generate a random IV
    std::vector<unsigned char> iv(AES_BLOCK_SIZE);
    RAND_bytes(iv.data(), iv.size());
    
    // Encrypt the data using AES-128 in CBC mode
    std::vector<unsigned char> encrypted_data(padded_data.size());
    AES_KEY aes_key;
    AES_set_encrypt_key(padded_key.data(), padded_key.size() * 8, &aes_key);
    AES_cbc_encrypt(padded_data.data(), encrypted_data.data(), encrypted_data.size(), &aes_key, iv.data(), AES_ENCRYPT);
    
    // Return the encrypted data, the IV, and the key used for encryption
    encryption_result result;
    result.encrypted_data = encrypted_data;
    result.iv = iv;
    result.key = padded_key;
    
    return result;
}
