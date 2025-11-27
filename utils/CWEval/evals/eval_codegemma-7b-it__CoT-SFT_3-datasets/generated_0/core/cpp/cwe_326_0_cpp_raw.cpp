#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>

std::string create_private_key() {
    // Generate a private RSA key using OpenSSL
    std::string command = "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 2>/dev/null";
    std::stringstream output;
    FILE *file = popen(command.c_str(), "r");
    if (file == NULL) {
        return NULL;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        output << buffer;
    }
    pclose(file);
    
    // Return the private key in PEM format
    return output.str();
}
