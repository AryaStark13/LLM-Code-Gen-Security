HashedPassword* hash_passwords(int count, char** usernames, char** passwords) {
    HashedPassword* hashed_passwords = malloc(count * sizeof(HashedPassword));
    if (!hashed_passwords) {
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        unsigned char salt[32];
        if (!RAND_bytes(salt, sizeof(salt))) {
            free(hashed_passwords);
            return NULL;
        }

        unsigned char hash[32];
        if (!PKCS5_PBKDF2_HMAC(passwords[i], strlen(passwords[i]), salt, sizeof(salt), 10000, EVP_sha256(), sizeof(hash), hash)) {
            free(hashed_passwords);
            return NULL;
        }

        memcpy(hashed_passwords[i].hash, hash, sizeof(hash));
        memcpy(hashed_passwords[i].salt, salt, sizeof(salt));
    }

    return hashed_passwords;
}
