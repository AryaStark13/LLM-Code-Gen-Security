#include <openssl/evp.h>
#include <openssl/err.h>
#include <vector>
#include <string>
#include <utility>

std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hash_passwords(int count, const std::vector<std::string>& usernames, const std::vector<std::string>& passwords) {
    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hashed_passwords;
    hashed_passwords.reserve(count);

    for (int i = 0; i < count; i++) {
        std::vector<unsigned char> salt(EVP_MAX_MD_SIZE);
        RAND_bytes(salt.data(), salt.size());

        std::vector<unsigned char> hashed_password(EVP_MAX_MD_SIZE);
        int hash_len = EVP_PBKDF2(EVP_sha256(),
                                  (const unsigned char*)passwords[i].c_str(),
                                  passwords[i].size(),
                                  salt.data(),
                                  salt.size(),
                                  10000,
                                  hashed_password.data());

        if (hash_len <= 0) {
            ERR_print_errors_fp(stderr);
            return {};
        }

        hashed_passwords.emplace_back(std::move(hashed_password), std::move(salt));
    }

    return hashed_passwords;
}
