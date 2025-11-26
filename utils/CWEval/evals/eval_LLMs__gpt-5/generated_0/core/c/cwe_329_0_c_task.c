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
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// Structure to hold the return values
typedef struct {
    unsigned char* encrypted_data;
    size_t encrypted_len;
    unsigned char* iv;
    unsigned char* key;
} encryption_result;

/**
 * Pad the given data using PKCS#7 padding to AES block size boundaries.
 *
 * @param data        The input data buffer to pad.
 * @param data_len    The length of the input data buffer in bytes.
 * @param padded_len  Output parameter that will receive the length of the padded buffer.
 *
 * @return A newly allocated buffer containing the padded data (must be freed by caller),
 *         or NULL on failure. On failure, padded_len is set to 0.
 */
char* pad_data(const unsigned char* data, size_t data_len, size_t* padded_len) {
    if (padded_len == NULL) {
        return NULL;
    }
    *padded_len = 0;

    // Validate input; if data_len > 0, data must not be NULL
    if (data_len > 0 && data == NULL) {
        return NULL;
    }

    const size_t block_size = AES_BLOCK_SIZE; // AES block size is 16 bytes
    size_t remainder = data_len % block_size;
    size_t pad_size = block_size - remainder;
    if (pad_size == 0) {
        pad_size = block_size; // Always add a full block of padding if already aligned
    }

    size_t out_len = data_len + pad_size;
    unsigned char* out = (unsigned char*)malloc(out_len);
    if (out == NULL) {
        return NULL;
    }

    // Copy original data
    if (data_len > 0) {
        memcpy(out, data, data_len);
    }

    // PKCS#7 padding: each padding byte equals the number of padding bytes
    unsigned char pad_val = (unsigned char)pad_size;
    for (size_t i = 0; i < pad_size; ++i) {
        out[data_len + i] = pad_val;
    }

    *padded_len = out_len;
    return (char*)out;
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

    // Basic input validation
    if (key == NULL || key_len == 0 || (data_len > 0 && data == NULL)) {
        return result;
    }

    // Determine AES key size to use: prefer 256-bit, fallback to 192 or 128 if needed.
    size_t key_bytes = 0;
    if (key_len >= 32) {
        key_bytes = 32; // AES-256
    } else if (key_len >= 24) {
        key_bytes = 24; // AES-192
    } else if (key_len >= 16) {
        key_bytes = 16; // AES-128
    } else {
        // Not enough key material
        return result;
    }
    int key_bits = (int)(key_bytes * 8);

    // Allocate and copy the used key prefix
    unsigned char* used_key = (unsigned char*)malloc(key_bytes);
    if (used_key == NULL) {
        return result;
    }
    memcpy(used_key, key, key_bytes);

    // Allocate and generate a random IV
    unsigned char* iv = (unsigned char*)malloc(AES_BLOCK_SIZE);
    if (iv == NULL) {
        free(used_key);
        return result;
    }
    if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        free(used_key);
        free(iv);
        return result;
    }

    // Pad the data
    size_t padded_len = 0;
    char* padded_buf = pad_data(data, data_len, &padded_len);
    if (padded_buf == NULL) {
        free(used_key);
        free(iv);
        return result;
    }
    unsigned char* padded = (unsigned char*)padded_buf;

    // Allocate output buffer
    unsigned char* enc_out = (unsigned char*)malloc(padded_len);
    if (enc_out == NULL) {
        free(used_key);
        free(iv);
        free(padded);
        return result;
    }

    // Set up AES key
    AES_KEY aes_key;
    if (AES_set_encrypt_key(used_key, key_bits, &aes_key) != 0) {
        free(used_key);
        free(iv);
        free(padded);
        free(enc_out);
        return result;
    }

    // Use a local copy of the IV for encryption since AES_cbc_encrypt updates it.
    unsigned char iv_enc[AES_BLOCK_SIZE];
    memcpy(iv_enc, iv, AES_BLOCK_SIZE);

    // Perform AES CBC encryption
    AES_cbc_encrypt(padded, enc_out, (unsigned int)padded_len, &aes_key, iv_enc, AES_ENCRYPT);

    // Clean up padded buffer
    free(padded);

    // Populate result
    result.encrypted_data = enc_out;
    result.encrypted_len = padded_len;
    result.iv = iv;
    result.key = used_key;

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
