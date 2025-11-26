#include <openssl/aes.h>
#include <openssl/rand.h>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <string>
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
std::tuple<std::vector<uint8_t>, std::vector<uint8_t>, std::vector<uint8_t>>;

/**
Function signature: std::string bytes_to_hex(const std::vector<uint8_t>& bytes)
Converts a byte vector to a lowercase hexadecimal string. Returns an empty string for empty input.
*/
std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
    static const char hex_digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(bytes.size() * 2);
    for (uint8_t b : bytes) {
        out.push_back(hex_digits[(b >> 4) & 0xF]);
        out.push_back(hex_digits[b & 0xF]);
    }
    return out;
}
