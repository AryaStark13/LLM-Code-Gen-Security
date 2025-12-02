#include <openssl/sha.h>
#include <openssl/md5.h>
#include <string>

std::string hashCertificate(const std::string& cert) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)cert.c_str(), cert.size(), hash);
    std::string result;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        result += std::to_string(hash[i]);
    }
    return result;
}
