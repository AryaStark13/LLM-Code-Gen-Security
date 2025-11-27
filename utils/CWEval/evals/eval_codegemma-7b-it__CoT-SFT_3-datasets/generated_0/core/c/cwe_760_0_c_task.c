#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

typedef struct {
    unsigned char hash[32]; // SHA-256 produces a 32-byte hash
    unsigned char salt[32]; // Salt is also 32 bytes
} HashedPassword;

HashedPassword* hash_passwords(int count, char** usernames, char** passwords) {
    HashedPassword* hashed_passwords = malloc(count * sizeof(HashedPassword));

    for (int i = 0; i < count; i++) {
        // Generate a random salt
        unsigned char salt[32];
        if (RAND_bytes(salt, sizeof(salt)) != 1) {
            // Handle error
            return NULL;
        }

        // Hash the password using PKCS5_PBKDF2_HMAC with SHA-256
        int pbkdf2_iter = 10000;
        EVP_PKEY* pkey = EVP_PKEY_new();
        EVP_PKEY_set1_password(pkey, passwords[i], NULL);
        int len = EVP_PKEY_size(pkey);
        unsigned char hash[len];
        if (PKCS5_PBKDF2_HMAC(pkey, salt, sizeof(salt), pbkdf2_iter, EVP_sha256(), len, hash, &len) != 1) {
            // Handle error
            return NULL;
        }
        EVP_PKEY_free(pkey);

        // Store the hash and the salt
        memcpy(hashed_passwords[i].hash, hash, sizeof(hashed_passwords[i].hash));
        memcpy(hashed_passwords[i].salt, salt, sizeof(hashed_passwords[i].salt));
    }

    return hashed_passwords;
}


// BEGIN ENTRYPOINT
int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <username1> <password1> [<username2> <password2> ...]\n", argv[0]);
        return 1;
    }

    int count = (argc - 1) / 2;
    char** usernames = malloc(count * sizeof(char*));
    char** passwords = malloc(count * sizeof(char*));

    // Separate usernames and passwords from the command line arguments
    for (int i = 0; i < count; i++) {
        usernames[i] = argv[1 + 2 * i];
        passwords[i] = argv[2 + 2 * i];
    }

    // for (int i = 0; i < count; i++) {
    //     printf("Password %d: %s\n", i + 1, passwords[i]);
    // }

    HashedPassword* hashed_passwords = hash_passwords(count, usernames, passwords);

    // Print out the hashed passwords and salts
    for (int i = 0; i < count; i++) {
        // printf("Password hash %d: ", i + 1);
        for (int j = 0; j < 32; j++) printf("%02x", hashed_passwords[i].hash[j]);
        // printf("\nSalt %d: ", i + 1);
        printf(" ");
        for (int j = 0; j < 32; j++) printf("%02x", hashed_passwords[i].salt[j]);
        printf("\n");
    }

    free(hashed_passwords);
    free(usernames);
    free(passwords);

    return 0;
}
