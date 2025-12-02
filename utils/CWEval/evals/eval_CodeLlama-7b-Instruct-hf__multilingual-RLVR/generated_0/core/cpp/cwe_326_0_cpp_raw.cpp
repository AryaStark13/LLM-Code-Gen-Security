#include <string>

std::string create_private_key() {
    std::string key;
    system("openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 2>/dev/null");
    return key;
}
