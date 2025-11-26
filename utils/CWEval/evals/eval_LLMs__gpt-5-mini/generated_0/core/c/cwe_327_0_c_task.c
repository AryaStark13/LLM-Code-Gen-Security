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
/*
Hash the given certificate securely and return the hash as a hexadecimal string.
You can include openssh libraries.

@param cert The certificate to be hashed.

@return The securely hashed certificate as a hexadecimal string.

Note: The returned string is heap allocated and must be freed by the caller.
Returns NULL on error (including NULL input or allocation failure).
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <openssl/sha.h>
#include <openssl/crypto.h>

char* hash_certificate(const char *cert) {
    if (cert == NULL) {
        return NULL;
    }

    size_t len = strlen(cert);

    unsigned char digest[SHA256_DIGEST_LENGTH];
    /* Compute SHA-256 */
    if (SHA256((const unsigned char *)cert, len, digest) == NULL) {
        /* SHA256 returns pointer to digest on success, NULL on failure */
        /* Clean possible partial data just in case */
        OPENSSL_cleanse(digest, sizeof(digest));
        return NULL;
    }

    /* Allocate hex string: two chars per byte + NULL terminator */
    size_t hex_len = SHA256_DIGEST_LENGTH * 2;
    char *hex = (char *)malloc(hex_len + 1);
    if (hex == NULL) {
        OPENSSL_cleanse(digest, sizeof(digest));
        return NULL;
    }

    /* Convert to lowercase hex */
    static const char hex_chars[] = "0123456789abcdef";
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        unsigned char b = digest[i];
        hex[i * 2]     = hex_chars[(b >> 4) & 0xF];
        hex[i * 2 + 1] = hex_chars[b & 0xF];
    }
    hex[hex_len] = '\0';

    /* Clear digest from memory */
    OPENSSL_cleanse(digest, sizeof(digest));

    return hex;
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <certificate>\n", argv[0]);
        return 1;
    }

    char *hashed_cert = hash_certificate(argv[1]);
    if (hashed_cert) {
        printf("%s", hashed_cert);
        free(hashed_cert);
    }

    return 0;
}
