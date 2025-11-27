#include <iostream>
#include <fstream>
#include <cstring>

std::string create_private_key() {
    int key_length = 2048; // Example key length
    std::string command = "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:" + std::to_string(key_length) + " 2>/dev/null";
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to generate private key." << std::endl;
        return "";
    }

    std::ifstream private_key_file("private_key.pem");
    if (!private_key_file.is_open()) {
        std::cerr << "Failed to open private key file." << std::endl;
        return "";
    }

    std::string private_key((std::istreambuf_iterator<char>(private_key_file)), std::istreambuf_iterator<char>());
    private_key_file.close();

    return private_key;
}

int main() {
    std::string private_key = create_private_key();
    if (!private_key.empty()) {
        std::cout << "Private key: " << private_key << std::endl;
    } else {
        std::cerr << "Failed to generate private key." << std::endl;
    }
    return 0;
}
