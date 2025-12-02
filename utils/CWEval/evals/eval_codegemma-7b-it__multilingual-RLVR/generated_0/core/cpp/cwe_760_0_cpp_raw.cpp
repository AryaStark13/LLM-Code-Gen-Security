#include <openssl/evp.h>
#include <vector>
#include <string>
#include <utility>

std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hash_passwords(int count, const std::vector<std::string>& usernames, const std::vector<std::string>& passwords) {
    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hashed_passwords;
    
    for (int i = 0; i < count; i++) {
        // Generate a salt of the same length as the output hash
        // In a real-world application, the salt should be generated securely
        std::vector<unsigned char> salt(EVP_MAX_MD_SIZE);
        
        // Hash the password using PKCS5_PBKDF2_HMAC
        std::vector<unsigned char> hashed_password(EVP_MAX_MD_SIZE);
        PKCS5_PBKDF2_HMAC(
            reinterpret_cast<const unsigned char*>(passwords[i].c_str()),
            reinterpret_cast<unsigned char*>(salt.data()),
            salt.size(),
            reinterpret_cast<unsigned char*>(hashed_password.data()),
            EVP_sha256(),
            10000
        );
        
        hashed_passwords.push_back({hashed_password, salt});
    }
    
    return hashed_passwords;
}
