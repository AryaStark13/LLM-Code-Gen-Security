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
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>

// Structure to hold the return values
typedef struct {
    unsigned char* encrypted_data;
    size_t encrypted_len;
    unsigned char* iv;
    unsigned char* key;
} encryption_result;

/**
 * Pad the input data using PKCS#7 padding for AES block size (16 bytes).
 *
 * @param data       The input data to be padded.
 * @param data_len   The length of the input data.
 * @param padded_len Output parameter to receive the length of the padded data.
 *
 * @return A newly allocated buffer containing the padded data, or NULL on error.
 *         The caller is responsible for freeing the returned buffer.
 */
char* pad_data(const unsigned char* data, size_t data_len, size_t* padded_len) {
    if (padded_len == NULL) {
        return NULL;
    }
    *padded_len = 0;

    if (data == NULL) {
        return NULL;
    }

    const size_t block_size = AES_BLOCK_SIZE; // 16 bytes for AES
    size_t pad_len = block_size - (data_len % block_size);
    if (pad_len == 0) {
        pad_len = block_size;
    }

    // Check for size_t overflow (conservative check)
    if (data_len > SIZE_MAX - pad_len) {
        return NULL;
    }

    size_t total_len = data_len + pad_len;
    unsigned char* padded = (unsigned char*)malloc(total_len);
    if (padded == NULL) {
        return NULL;
    }

    if (data_len > 0) {
        memcpy(padded, data, data_len);
    }
    memset(padded + data_len, (int)pad_len, pad_len);

    *padded_len = total_len;
    return (char*)padded;
}

/**
Encrypt the data with the key using CBC mode.
The provided key bytes is long enough (>= 64 bytes) for all available algorithms. Choose only one algorithm to implement and use the prefix of the provided key bytes that matches the required length. Do NOT choose the algorithm based on the length of the key.
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

    // Validate inputs
    if (key == NULL || data == NULL) {
        return result;
    }

    // Algorithm chosen: AES-256-CBC
    const size_t aes_key_bytes = 32;         // 256-bit key
    const size_t iv_len = AES_BLOCK_SIZE;    // 16 bytes

    if (key_len < aes_key_bytes) {
        // Provided key is insufficient for AES-256
        return result;
    }

    // Copy the first 32 bytes of the provided key
    unsigned char* key_used = (unsigned char*)malloc(aes_key_bytes);
    if (key_used == NULL) {
        return result;
    }
    memcpy(key_used, key, aes_key_bytes);

    // Allocate and generate IV
    unsigned char* iv = (unsigned char*)malloc(iv_len);
    if (iv == NULL) {
        free(key_used);
        return result;
    }
    if (RAND_bytes(iv, (int)iv_len) != 1) {
        free(key_used);
        free(iv);
        return result;
    }

    // Pad the input data
    size_t padded_len = 0;
    char* padded_data_char = pad_data(data, data_len, &padded_len);
    if (padded_data_char == NULL || padded_len == 0) {
        free(key_used);
        free(iv);
        return result;
    }
    unsigned char* padded_data = (unsigned char*)padded_data_char;

    // Allocate output buffer
    unsigned char* encrypted = (unsigned char*)malloc(padded_len);
    if (encrypted == NULL) {
        free(key_used);
        free(iv);
        free(padded_data);
        return result;
    }

    // Prepare AES key
    AES_KEY aes_key;
    if (AES_set_encrypt_key(key_used, 256, &aes_key) != 0) {
        free(key_used);
        free(iv);
        free(padded_data);
        free(encrypted);
        return result;
    }

    // AES_cbc_encrypt mutates the IV; use a working copy
    unsigned char iv_work[AES_BLOCK_SIZE];
    memcpy(iv_work, iv, iv_len);

    // Perform encryption
    AES_cbc_encrypt(padded_data, encrypted, padded_len, &aes_key, iv_work, AES_ENCRYPT);

    // Clean up padded data
    free(padded_data);

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
