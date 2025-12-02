#include <openssl/aes.h>
#include <openssl/rand.h>
#include <vector>
#include <tuple>

std::vector<uint8_t> pkcs7_pad(const std::vector<uint8_t>& data, size_t block_size) {
    size_t padding_len = block_size - (data.size() % block_size);
    std::vector<uint8_t> padded = data;
    padded.insert(padded.end(), padding_len, static_cast<uint8_t>(padding_len));
    return padded;
}

std::tuple<std::vector<uint8_t>, std::vector<uint8_t>, std::vector<uint8_t>>
encrypt_data(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) {
    // Ensure the key is at least 64 bytes long
    if (key.size() < 64) {
        // Handle the case where the key is too short
        return {std::vector<uint8_t>(), std::vector<uint8_t>(), key};
    }

    // Generate a random IV
    std::vector<uint8_t> iv(AES_BLOCK_SIZE);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        // Handle the case where the IV generation fails
        return {std::vector<uint8_t>(), std::vector<uint8_t>(), key};
    }

    // Pad the data
    std::vector<uint8_t> padded_data = pkcs7_pad(data, AES_BLOCK_SIZE);

    // Encrypt the data
    std::vector<uint8_t> encrypted_data(padded_data.size());
    AES_KEY aes_key;
    if (AES_set_encrypt_key(&key[0], key.size() * 8, &aes_key) != 0) {
        // Handle the case where the key setup fails
        return {std::vector<uint8_t>(), std::vector<uint8_t>(), key};
    }
    AES_cbc128_encrypt(padded_data.data(), encrypted_data.data(), encrypted_data.size(), &aes_key, iv.data(), AES_ENCRYPT);

    return {encrypted_data, iv, key};
}
