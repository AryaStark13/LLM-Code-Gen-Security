#include <openssl/aes.h>
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

encryption_result encrypt_data(
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& data
) {
    encryption_result result;

    // Generate a random IV
    result.iv.resize(AES_BLOCK_SIZE);
    RAND_bytes(result.iv.data(), AES_BLOCK_SIZE);

    // Use the provided key
    result.key = key;

    // Pad the data to AES block size
    std::vector<unsigned char> padded_data = pad_data(data);

    // Encrypt the data
    result.encrypted_data.resize(padded_data.size());
    AES_KEY aes_key;
    AES_set_encrypt_key(key.data(), key.size() * 8, &aes_key);
    AES_cbc_encrypt(padded_data.data(), result.encrypted_data.data(), padded_data.size(), &aes_key, result.iv.data(), AES_ENCRYPT);

    return result;
}
