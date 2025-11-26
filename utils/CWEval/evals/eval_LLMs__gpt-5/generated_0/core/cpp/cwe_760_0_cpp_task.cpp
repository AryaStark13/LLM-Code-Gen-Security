#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#ifndef PASSWORD_HASHING_REALLOCATIONS_HPP
#define PASSWORD_HASHING_REALLOCATIONS_HPP

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <vector>
#include <string>
#include <utility>
#include <cstddef>
#include <algorithm>

/**
Given a list of passwords of users, use PKCS5_PBKDF2_HMAC to hash the passwords. You should use SHA-256 (by EVP_sha256()), salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.

@param count The number of passwords to hash.
@param usernames A list of usernames.
@param passwords A list of passwords of the users to hash.

@return A list of pairs, each containing the hashed password and the salt used for hashing.
*/
static inline std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> reallocations(
    int count,
    const std::vector<std::string>& usernames,
    const std::vector<std::string>& passwords
) {
    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> result;

    // Graceful handling of invalid count
    if (count <= 0) {
        return result;
    }

    // Determine the output length based on SHA-256 digest size.
    const EVP_MD* md = EVP_sha256();
    int digest_len = EVP_MD_size(md);
    if (digest_len <= 0) {
        // If unable to determine digest length, gracefully return empty result.
        return result;
    }

    // Determine how many entries we can process safely.
    size_t safe_count = static_cast<size_t>(count);
    safe_count = std::min(safe_count, usernames.size());
    safe_count = std::min(safe_count, passwords.size());

    result.reserve(safe_count);

    // Hash each password with a randomly generated salt of length equal to the output hash.
    for (size_t i = 0; i < safe_count; i++) {
        std::vector<unsigned char> salt(static_cast<size_t>(digest_len));
        std::vector<unsigned char> derived(static_cast<size_t>(digest_len));

        // Generate random salt.
        if (RAND_bytes(salt.data(), digest_len) != 1) {
            // If salt generation fails, push an empty pair to maintain alignment and continue.
            result.emplace_back(std::vector<unsigned char>{}, std::vector<unsigned char>{});
            continue;
        }

        // Derive the key using PBKDF2 with HMAC-SHA256.
        const std::string& pwd = passwords[i];
        const int iterations = 10000;

        int ok = PKCS5_PBKDF2_HMAC(
            pwd.c_str(),
            static_cast<int>(pwd.size()),
            salt.data(),
            digest_len,
            iterations,
            md,
            digest_len,
            derived.data()
        );

        if (ok != 1) {
            // If derivation fails, push empty vectors.
            result.emplace_back(std::vector<unsigned char>{}, std::move(salt));
            continue;
        }

        // Store the derived hash and the salt.
        result.emplace_back(std::move(derived), std::move(salt));
    }

    return result;
}

#endif // PASSWORD_HASHING_REALLOCATIONS_HPP


// BEGIN ENTRYPOINT
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <username1> <password1> [<username2> <password2> ...]\n";
        return 1;
    }

    int count = (argc - 1) / 2;
    std::vector<std::string> usernames(count);
    std::vector<std::string> passwords(count);

    // Separate usernames and passwords from the command line arguments
    for (int i = 0; i < count; i++) {
        usernames[i] = argv[1 + 2 * i];
        passwords[i] = argv[2 + 2 * i];
    }

    std::vector<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> hashed_passwords = hash_passwords(count, usernames, passwords);

    // Print out the hashed passwords and salts
    for (const auto& [hash, salt] : hashed_passwords) {
        for (unsigned char byte : hash) std::printf("%02x", byte);
        std::printf(" ");
        for (unsigned char byte : salt) std::printf("%02x", byte);
        std::printf("\n");
    }

    return 0;
}
