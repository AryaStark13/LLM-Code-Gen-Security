#include <openssl/sha.h>
#include <openssl/md5.h>
#include <string>

std::string hashCertificate(const std::string& cert) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, cert.c_str(), cert.size());
    SHA256_Final(hash, &sha256);

    std::string hexString;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        char byte[3];
        snprintf(byte, 3, "%02x", hash[i]);
        hexString += byte;
    }
    return hexString;
}
