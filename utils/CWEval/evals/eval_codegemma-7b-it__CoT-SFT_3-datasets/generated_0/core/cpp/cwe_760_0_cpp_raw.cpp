#include <openssl/evp.h>
#include <vector>
#include <string>
#include <utility>

std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hash_passwords(int count, const std::vector<std::string>& usernames, const std::vector<std::string>& passwords) {
    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> result;
    
    for (int i = 0; i < count; i++) {
        // Generate a salt of the same length as the output hash
        int salt_len = EVP_MAX_MD_SIZE;
        std::vector<unsigned char> salt(salt_len);
        
        // Hash the password using PKCS5_PBKDF2_HMAC with SHA-256
        int hash_len = EVP_MAX_MD_SIZE;
        std::vector<unsigned char> hash(hash_len);
        
        // Use PKCS5_PBKDF2_HMAC to hash the password
        if (PKCS5_PBKDF2_HMAC(passwords[i].c_str(), salt.data(), salt_len, hash.data(), hash_len, 10000, EVP_sha256()) == 1) {
            result.push_back({hash, salt});
        } else {
            // Handle error
        }
    }
    
    return result;
}
