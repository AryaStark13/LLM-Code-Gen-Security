#include <openssl/evp.h>
#include <openssl/rand.h>
#include <vector>
#include <string>

std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hash_passwords(int count, const std::vector<std::string>& usernames, const std::vector<std::string>& passwords) {
    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hashed_passwords;

    for (int i = 0; i < count; i++) {
        std::vector<unsigned char> salt(EVP_MAX_MD_SIZE);
        if (RAND_bytes(salt.data(), salt.size()) <= 0) {
            // Handle error
            return hashed_passwords;
        }

        std::vector<unsigned char> hash(EVP_MAX_MD_SIZE);
        if (PKCS5_PBKDF2_HMAC(passwords[i].c_str(), passwords[i].size(), salt.data(), salt.size(), EVP_sha256(), 10000, hash.data(), hash.size()) <= 0) {
            // Handle error
            return hashed_passwords;
        }

        hashed_passwords.push_back(std::make_pair(hash, salt));
    }

    return hashed_passwords;
}
