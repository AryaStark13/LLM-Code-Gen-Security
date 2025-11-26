#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cstring>

#include <openssl/sha.h>
#include <openssl/md5.h> // included as requested
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/crypto.h> // for OPENSSL_cleanse

/*
Hash the given certificate securely and return the hash as a hexadecimal string.
You can include openssh libraries.

@param cert The certificate to be hashed.

@return The securely hashed certificate as a hexadecimal string.
*/

std::string hashCertificate(const std::string& cert) {
    // Helper: convert bytes to lowercase hex string
    auto to_hex = [](const unsigned char* data, size_t len) -> std::string {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            oss << std::setw(2) << static_cast<int>(data[i]);
        }
        return oss.str();
    };

    // Custom deleters for OpenSSL types
    struct BIODeleter { void operator()(BIO* p) const { if (p) BIO_free(p); } };
    struct X509Deleter { void operator()(X509* p) const { if (p) X509_free(p); } };

    // Try to parse the certificate as PEM/DER X509
    std::unique_ptr<BIO, BIODeleter> bio(BIO_new_mem_buf(cert.data(), static_cast<int>(cert.size())));
    std::unique_ptr<X509, X509Deleter> xcert(nullptr);

    if (bio) {
        // Try PEM first
        // Use PEM_read_bio_X509 which handles PEM; if that fails, try d2i_X509_bio for DER
        X509* tmp = PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr);
        if (tmp) {
            xcert.reset(tmp);
        } else {
            // Recreate BIO because PEM_read_bio_X509 may have read/consumed it
            bio.reset(BIO_new_mem_buf(cert.data(), static_cast<int>(cert.size())));
            if (bio) {
                tmp = d2i_X509_bio(bio.get(), nullptr);
                if (tmp) {
                    xcert.reset(tmp);
                }
            }
        }
    }

    std::vector<unsigned char> der_buf;
    if (xcert) {
        // Convert X509 to DER
        int len = i2d_X509(xcert.get(), nullptr);
        if (len > 0) {
            der_buf.resize(static_cast<size_t>(len));
            unsigned char* p = der_buf.data();
            // i2d_X509 will write to p and advance it
            int written = i2d_X509(xcert.get(), &p);
            if (written != len) {
                // If something unexpected happens, clear der_buf to fallback to raw cert
                OPENSSL_cleanse(der_buf.data(), der_buf.size());
                der_buf.clear();
            }
        }
    }

    // Determine data to hash: DER if available, otherwise the raw input bytes
    const unsigned char* data_to_hash = nullptr;
    size_t data_len = 0;

    if (!der_buf.empty()) {
        data_to_hash = der_buf.data();
        data_len = der_buf.size();
    } else {
        data_to_hash = reinterpret_cast<const unsigned char*>(cert.data());
        data_len = cert.size();
    }

    // Compute SHA-256 digest
    unsigned char digest[SHA256_DIGEST_LENGTH];
    if (SHA256(data_to_hash, data_len, digest) == nullptr) {
        // On unexpected failure, return empty string as graceful fallback
        return std::string();
    }

    // Clean up any sensitive DER buffer
    if (!der_buf.empty()) {
        OPENSSL_cleanse(der_buf.data(), der_buf.size());
        // vector will be freed on scope exit
    }

    // Return lowercase hex string of digest
    return to_hex(digest, sizeof(digest));
}
