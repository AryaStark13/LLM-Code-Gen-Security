HashedPassword* hash_passwords(int count, char** usernames, char** passwords) {
    HashedPassword* hashed_passwords = malloc(count * sizeof(HashedPassword));

    for (int i = 0; i < count; i++) {
        // Generate a random salt
        RAND_bytes(hashed_passwords[i].salt, 32);

        // Use PKCS5_PBKDF2_HMAC to hash the password
        PKCS5_PBKDF2_HMAC(passwords[i], strlen(passwords[i]), hashed_passwords[i].salt, 32, EVP_sha256(), 10000, hashed_passwords[i].hash, 32);
    }

    return hashed_passwords;
}
