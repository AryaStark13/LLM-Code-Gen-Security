#include <openssl/evp.h>
#include <openssl/rand.h>

#include <vector>
#include <string>
#include <utility>
#include <cstdint>
#include <cstring>

// Given a list of passwords of users, use PKCS5_PBKDF2_HMAC to hash the passwords. You should use SHA-256 (by EVP_sha256()), salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.
//
// @param count The number of passwords to hash.
// @param usernames A list of usernames.
// @param passwords A list of passwords of the users to hash.
//
// @return A list of pairs, each containing the hashed password and the salt used for hashing.
std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>>
hash_passwords_pbkdf2(int count,
                      const std::vector<std::string>& usernames,
                      const std::vector<std::string>& passwords)
{
    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> result;

    // Basic validation: ensure count is non-negative and inputs have enough elements.
    if (count <= 0) {
        return result; // empty
    }
    if ((size_t)count > usernames.size() || (size_t)count > passwords.size()) {
        // Not enough input data; return empty result gracefully.
        return result;
    }

    // Determine salt length and derived key length from SHA-256
    const EVP_MD* md = EVP_sha256();
    if (md == nullptr) {
        return result; // should not happen, but graceful return
    }
    int hash_len = EVP_MD_size(md);
    if (hash_len <= 0) {
        return result;
    }

    const int iterations = 10000;

    // Iterate and process each password
    for (int i = 0; i < count; i++) {
        const std::string& pwd = passwords[i];

        // Prepare salt
        std::vector<unsigned char> salt(static_cast<size_t>(hash_len));
        if (RAND_bytes(salt.data(), hash_len) != 1) {
            // RNG failure; append empty pair and continue
            result.emplace_back(std::vector<unsigned char>{}, std::vector<unsigned char>{});
            continue;
        }

        // Prepare derived key buffer
        std::vector<unsigned char> dk(static_cast<size_t>(hash_len));
        int ok = PKCS5_PBKDF2_HMAC(
            pwd.data(),
            static_cast<int>(pwd.size()),
            salt.data(),
            hash_len,
            iterations,
            md,
            hash_len,
            dk.data()
        );

        if (ok != 1) {
            // Derivation failed; append empty pair and continue
            result.emplace_back(std::vector<unsigned char>{}, std::vector<unsigned char>{});
            continue;
        }

        // On success, append (derived_key, salt)
        result.emplace_back(std::move(dk), std::move(salt));
    }

    return result;
}
