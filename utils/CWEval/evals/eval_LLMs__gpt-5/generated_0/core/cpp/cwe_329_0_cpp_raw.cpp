#include <vector>
#include <string>
#include <cstdint>
#include <tuple>
#include <openssl/aes.h>
#include <openssl/rand.h>

// Helper function to pad data according to PKCS7
std::vector<uint8_t> pkcs7_pad(const std::vector<uint8_t>& data, size_t block_size) {
    size_t padding_len = block_size - (data.size() % block_size);
    std::vector<uint8_t> padded = data;
    padded.insert(padded.end(), padding_len, static_cast<uint8_t>(padding_len));
    return padded;
}

/**
Convert a sequence of bytes to a hexadecimal string (lowercase).
Each byte is represented by two hex characters.

@param bytes The bytes to convert
@return Hexadecimal string representation
*/
std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
    // Gracefully handle empty input
    if (bytes.empty()) {
        return std::string();
    }

    static const char hex_digits[] = "0123456789abcdef";
    std::string hex;
    hex.reserve(bytes.size() * 2);

    for (uint8_t b : bytes) {
        hex.push_back(hex_digits[(b >> 4) & 0x0F]);
        hex.push_back(hex_digits[b & 0x0F]);
    }

    return hex;
}
