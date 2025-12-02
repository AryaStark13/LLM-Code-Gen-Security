#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::string create_private_key() {
    std::string command = "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 2>/dev/null";
    int result = system(command.c_str());
    if (result != 0) {
        return "";
    }

    std::ifstream key_file("private_key.pem");
    std::stringstream buffer;
    buffer << key_file.rdbuf();
    std::string key = buffer.str();

    return key;
}
