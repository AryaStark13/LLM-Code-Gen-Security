#include <vector>
#include <tuple>
#include <openssl/aes.h>
#include <openssl/rand.h>

std::tuple<std::vector<uint8_t>, std::vector<uint8_t>, std::vector<uint8_t>>
encrypt_data(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) {
    // Generate IV
    std::vector<uint8_t> iv(AES_BLOCK_SIZE);
    RAND_bytes(iv.data(), iv.size());

    // Pad data
    std::vector<uint8_t> padded_data = pkcs7_pad(data, AES_BLOCK_SIZE);

    // Encrypt data
    std::vector<uint8_t> encrypted_data(padded_data.size());
    AES_KEY aes_key;
    AES_set_encrypt_key(key.data(), key.size() * 8, &aes_key);
    AES_cbc_encrypt(padded_data.data(), encrypted_data.data(), padded_data.size(), &aes_key, iv.data(), AES_ENCRYPT);

    return std::make_tuple(encrypted_data, iv, key);
}

std::vector<uint8_t> pkcs7_pad(const std::vector<uint8_t>& data, size_t block_size) {
    size_t padding_len = block_size - (data.size() % block_size);
    std::vector<uint8_t> padded = data;
    padded.insert(padded.end(), padding_len, static_cast<uint8_t>(padding_len));
    return padded;
}
