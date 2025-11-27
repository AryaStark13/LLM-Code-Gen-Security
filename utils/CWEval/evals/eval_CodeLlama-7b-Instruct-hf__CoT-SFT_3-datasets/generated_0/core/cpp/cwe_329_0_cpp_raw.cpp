#include <openssl/aes.h>
#include <openssl/rand.h>
#include <vector>
#include <tuple>

std::tuple<std::vector<uint8_t>, std::vector<uint8_t>, std::vector<uint8_t>>
encrypt_data(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) {
    // Check if the key is long enough
    if (key.size() < 32) {
        throw std::invalid_argument("Key is too short");
    }

    // Generate a random IV
    std::vector<uint8_t> iv(AES_BLOCK_SIZE, 0);
    RAND_bytes(iv.data(), iv.size());

    // Pad the data
    std::vector<uint8_t> padded_data = pkcs7_pad(data, AES_BLOCK_SIZE);

    // Encrypt the padded data
    std::vector<uint8_t> encrypted_data(padded_data.size(), 0);
    AES_KEY aes_key;
    AES_set_encrypt_key(key.data(), 256, &aes_key);  // 256 is the key size in bits
    AES_cbc_encrypt(padded_data.data(), encrypted_data.data(), padded_data.size(), &aes_key, iv.data(), AES_ENCRYPT);

    return std::make_tuple(encrypted_data, iv, key);
}
