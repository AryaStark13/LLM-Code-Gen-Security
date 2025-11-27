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


// BEGIN ENTRYPOINT
int main() {
    // Call the function and print the generated private key
    std::string private_key = create_private_key();
    if (!private_key.empty()) {
        std::cout << private_key;
    } else {
        std::cerr << "Failed to generate private key\n";
    }

    return 0;
}
