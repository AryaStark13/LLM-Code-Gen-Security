#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

/*
Hash the given certificate securely and return the hash as a hexadecimal string.
You can include openssh libraries.

@param cert The certificate to be hashed.

@return The securely hashed certificate as a hexadecimal string.
*/
std::string hashCertificate(const std::string& cert) {
    // Prefer using EVP interface with SHA-256 for secure hashing
    unsigned char md_buffer[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx != nullptr) {
        bool ok = true;
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
            ok = false;
        }
        if (ok && EVP_DigestUpdate(ctx, reinterpret_cast<const unsigned char*>(cert.data()), cert.size()) != 1) {
            ok = false;
        }
        if (ok && EVP_DigestFinal_ex(ctx, md_buffer, &md_len) != 1) {
            ok = false;
        }
        EVP_MD_CTX_free(ctx);

        if (ok) {
            std::ostringstream oss;
            oss << std::hex << std::nouppercase << std::setfill('0');
            for (unsigned int i = 0; i < md_len; ++i) {
                oss << std::setw(2) << static_cast<unsigned int>(md_buffer[i]);
            }
            return oss.str();
        }
    }

    // Fallback to direct SHA-256 if EVP interface is unavailable or fails
    unsigned char sha256_digest[SHA256_DIGEST_LENGTH];
    if (SHA256(reinterpret_cast<const unsigned char*>(cert.data()),
               static_cast<size_t>(cert.size()),
               sha256_digest) == nullptr) {
        // Gracefully return an empty string on failure
        return std::string();
    }

    // Convert digest to hexadecimal string
    std::ostringstream oss;
    oss << std::hex << std::nouppercase << std::setfill('0');
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(sha256_digest[i]);
    }
    return oss.str();
}
