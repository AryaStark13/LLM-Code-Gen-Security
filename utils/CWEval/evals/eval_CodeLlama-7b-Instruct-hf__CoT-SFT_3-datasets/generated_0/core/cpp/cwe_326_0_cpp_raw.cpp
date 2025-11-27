#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

std::string create_private_key() {
    // Generate a private RSA key using OpenSSL
    std::string command = "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 2>/dev/null";
    std::string key;
    char* output = NULL;
    size_t length = 0;

    // Execute the command and capture the output
    if (system(command.c_str()) == 0) {
        // Get the output length
        length = strlen(output);

        // Allocate memory for the key
        key = std::string(output, length);

        // Free the memory
        free(output);
    }

    return key;
}
