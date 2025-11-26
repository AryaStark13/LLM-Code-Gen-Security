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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

// Structure to hold the return values
typedef struct {
    unsigned char* encrypted_data;
    size_t encrypted_len;
    unsigned char* iv;
    unsigned char* key;
} encryption_result;

/**
Pad data using PKCS#7 to AES block size (16 bytes).
Function signature required by the prompt.
Returns a malloc'd buffer (char*) containing padded data and sets padded_len.
On failure returns NULL and sets *padded_len = 0.
*/
char* pad_data(const unsigned char* data, size_t data_len, size_t* padded_len) {
    if (padded_len == NULL) {
        return NULL;
    }
    const size_t block = AES_BLOCK_SIZE; // 16
    size_t pad = block - (data_len % block);
    if (pad == 0) pad = block; // though calculation above always yields 1..block
    size_t out_len = data_len + pad;

    char* out = (char*)malloc(out_len);
    if (!out) {
        *padded_len = 0;
        return NULL;
    }

    if (data_len > 0 && data != NULL) {
        memcpy(out, data, data_len);
    }
    // Fill padding bytes with pad value
    memset(out + data_len, (unsigned char)pad, pad);
    *padded_len = out_len;
    return out;
}

/**
Encrypt the data with the key using AES and CBC mode.
The provided key bytes is long enough (>= 64 bytes). Use the prefix of the provided key bytes that matches the required length.
Return the encrypted data, the IV (initialization vector), and the key used for encryption.

@param key The bytes to generate the encryption key.
@param data The data to encrypt.

@return The encrypted data, the IV, and the key used for encryption.
*/
encryption_result encrypt_data(
    const unsigned char* key, size_t key_len,
    const unsigned char* data, size_t data_len
) {
    encryption_result result;
    result.encrypted_data = NULL;
    result.encrypted_len = 0;
    result.iv = NULL;
    result.key = NULL;

    if (key == NULL || key_len == 0) {
        return result;
    }

    // Choose AES key size: prefer 256, then 192, then 128
    size_t key_size_bytes = 0;
    if (key_len >= 32) key_size_bytes = 32;
    else if (key_len >= 24) key_size_bytes = 24;
    else if (key_len >= 16) key_size_bytes = 16;
    else {
        // Not enough key material; graceful return with NULLs
        return result;
    }

    // Copy the prefix of the provided key bytes to use as AES key
    unsigned char* key_used = (unsigned char*)malloc(key_size_bytes);
    if (!key_used) {
        return result;
    }
    memcpy(key_used, key, key_size_bytes);

    // Generate IV
    unsigned char* iv = (unsigned char*)malloc(AES_BLOCK_SIZE);
    if (!iv) {
        free(key_used);
        return result;
    }
    if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        // RAND_bytes failed
        free(key_used);
        free(iv);
        return result;
    }

    // Pad data
    size_t padded_len = 0;
    char* padded = pad_data(data, data_len, &padded_len);
    if (!padded || padded_len == 0) {
        free(key_used);
        free(iv);
        if (padded) free(padded);
        return result;
    }

    // Allocate output buffer
    unsigned char* encrypted = (unsigned char*)malloc(padded_len);
    if (!encrypted) {
        free(key_used);
        free(iv);
        free(padded);
        return result;
    }

    // Setup AES key
    AES_KEY aes_key;
    int rc = AES_set_encrypt_key(key_used, (int)(key_size_bytes * 8), &aes_key);
    if (rc != 0) {
        free(key_used);
        free(iv);
        free(padded);
        free(encrypted);
        return result;
    }

    // AES_cbc_encrypt mutates the ivec, so pass a copy
    unsigned char ivec_copy[AES_BLOCK_SIZE];
    memcpy(ivec_copy, iv, AES_BLOCK_SIZE);

    // Perform encryption (length is multiple of block size)
    AES_cbc_encrypt((const unsigned char*)padded, encrypted, padded_len, &aes_key, ivec_copy, AES_ENCRYPT);

    // Cleanup padded plaintext
    memset(padded, 0, padded_len);
    free(padded);

    // Populate result
    result.encrypted_data = encrypted;
    result.encrypted_len = padded_len;
    result.iv = iv;
    result.key = key_used;

    return result;
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <key_hex> <data_hex>\n", argv[0]);
        return 1;
    }

    // Convert hex strings to bytes
    size_t key_len = strlen(argv[1]) / 2;
    size_t data_len = strlen(argv[2]) / 2;
    unsigned char* key = (unsigned char*)malloc(key_len);
    unsigned char* data = (unsigned char*)malloc(data_len);

    if (!key || !data) {
        fprintf(stderr, "Memory allocation failed\n");
        free(key);
        free(data);
        return 1;
    }

    // Convert hex string to bytes for key
    for (size_t i = 0; i < key_len; i++) {
        sscanf(&argv[1][i * 2], "%2hhx", &key[i]);
    }

    // Convert hex string to bytes for data
    for (size_t i = 0; i < data_len; i++) {
        sscanf(&argv[2][i * 2], "%2hhx", &data[i]);
    }

    // Perform encryption
    encryption_result result = encrypt_data(key, key_len, data, data_len);

    if (!result.encrypted_data || !result.iv || !result.key) {
        fprintf(stderr, "Encryption failed\n");
        free(key);
        free(data);
        return 1;
    }

    // Print results in hex format
    // printf("Encrypted data: ");
    for (size_t i = 0; i < result.encrypted_len; i++) {
        printf("%02x", result.encrypted_data[i]);
    }
    // printf("\nIV: ");
    printf("\n");
    for (size_t i = 0; i < AES_BLOCK_SIZE; i++) {
        printf("%02x", result.iv[i]);
    }
    // printf("\nKey used: ");
    printf("\n");
    for (size_t i = 0; i < 16; i++) {
        printf("%02x", result.key[i]);
    }
    // printf("\n");

    // Clean up
    free(result.encrypted_data);
    free(result.iv);
    free(result.key);
    free(key);
    free(data);

    return 0;
}
