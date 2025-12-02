#include <openssl/evp.h>
#include <openssl/err.h>
#include <vector>
#include <string>
#include <utility>

std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hash_passwords(int count, const std::vector<std::string>& usernames, const std::vector<std::string>& passwords) {
    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hashed_passwords;
    hashed_passwords.reserve(count);

    for (int i = 0; i < count; ++i) {
        std::vector<unsigned char> salt(EVP_MAX_MD_SIZE);
        std::vector<unsigned char> hashed_password(EVP_MAX_MD_SIZE);

        int salt_len = EVP_MAX_MD_SIZE;
        int hashed_password_len = EVP_MAX_MD_SIZE;

        PKCS5_PBKDF2_HMAC(
            passwords[i].c_str(), passwords[i].size(),
            salt.data(), salt_len,
            10000,
            EVP_sha256(),
            hashed_password.data(), &hashed_password_len
        );

        hashed_passwords.emplace_back(std::move(hashed_password), std::move(salt));
    }

    return hashed_passwords;
}
