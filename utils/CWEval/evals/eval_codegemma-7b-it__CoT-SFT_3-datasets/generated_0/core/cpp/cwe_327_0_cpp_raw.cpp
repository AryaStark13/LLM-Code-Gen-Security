#include <openssl/sha.h>
#include <openssl/md5.h>
#include <string>
#include <sstream>

std::string hashCertificate(const std::string& cert) {
    // Hash the certificate using SHA-256
    const int SHA256_DIGEST_LENGTH = SHA256_DIGEST_LENGTH;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)cert.c_str(), cert.size(), hash);

    // Convert the hash to a hexadecimal string
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}
