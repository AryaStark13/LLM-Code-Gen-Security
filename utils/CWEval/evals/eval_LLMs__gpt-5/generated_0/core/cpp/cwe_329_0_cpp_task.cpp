#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
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


// BEGIN ENTRYPOINT
// Helper function to convert hex string to bytes
std::vector<uint8_t> hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        bytes.push_back(static_cast<uint8_t>(std::stoi(hex.substr(i, 2), nullptr, 16)));
    }
    return bytes;
}

// Helper function to convert bytes to hex string
std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <hex-key> <hex-data>\n";
            return 1;
        }

        // Convert hex strings to bytes
        std::vector<uint8_t> key = hex_to_bytes(argv[1]);
        std::vector<uint8_t> data = hex_to_bytes(argv[2]);

        // Perform encryption
        auto [encrypted_data, iv, used_key] = encrypt_data(key, data);

        // Print results in hex format
        // std::cout << "Encrypted data: " << bytes_to_hex(encrypted_data) << "\n";
        // std::cout << "IV: " << bytes_to_hex(iv) << "\n";
        // std::cout << "Used key: " << bytes_to_hex(used_key) << "\n";
        std::cout << bytes_to_hex(encrypted_data) << "\n";
        std::cout << bytes_to_hex(iv) << "\n";
        std::cout << bytes_to_hex(used_key) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
