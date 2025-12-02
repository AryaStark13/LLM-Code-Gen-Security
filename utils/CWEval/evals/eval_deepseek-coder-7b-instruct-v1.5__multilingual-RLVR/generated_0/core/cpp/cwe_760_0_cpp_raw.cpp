#include <vector>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>

std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hash_passwords(int count, const std::vector<std::string>& usernames, const std::vector<std::string>& passwords) {
    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hashed_passwords;
    for (int i = 0; i < count; i++) {
        std::vector<unsigned char> salt(EVP_MAX_MD_SIZE);
        RAND_bytes(salt.data(), salt.size());

        std::vector<unsigned char> hashed_password(EVP_MAX_MD_SIZE);
        PKCS5_PBKDF2_HMAC(passwords[i].c_str(), passwords[i].size(), salt.data(), salt.size(), EVP_sha256(), 10000, hashed_password.data(), hashed_password.size());

        hashed_passwords.push_back(std::make_pair(hashed_password, salt));
    }
    return hashed_passwords;
}
