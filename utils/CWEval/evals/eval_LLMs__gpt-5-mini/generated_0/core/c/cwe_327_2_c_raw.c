#include <stdlib.h>
#include <string.h>
#include <stddef.h>
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
 * PKCS#7 pad the input data to AES block size (16).
 * Returns a newly malloc'ed buffer containing the padded data,
 * and sets padded_len. Returns NULL on failure.
 *
 * Function signature required by the prompt:
 * char* pad_data(const unsigned char* data, size_t data_len, size_t* padded_len)
 */
char* pad_data(const unsigned char* data, size_t data_len, size_t* padded_len) {
    if (padded_len == NULL) return NULL;

    const size_t block = AES_BLOCK_SIZE; // 16
    size_t pad_len = block - (data_len % block);
    if (pad_len == 0) pad_len = block; // always add padding

    size_t total = data_len + pad_len;
    unsigned char* out = (unsigned char*)malloc(total);
    if (!out) return NULL;

    if (data_len > 0 && data != NULL) {
        memcpy(out, data, data_len);
    }
    // Fill padding bytes with value = pad_len
    memset(out + data_len, (unsigned char)pad_len, pad_len);

    *padded_len = total;
    return (char*)out;
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

    // Choose AES-256-CBC explicitly and use first 32 bytes of provided key.
    const size_t AES256_KEY_LEN = 32;

    if (key == NULL || key_len < AES256_KEY_LEN || data == NULL) {
        // invalid inputs
        return result;
    }

    // Copy the key prefix to return
    unsigned char* used_key = (unsigned char*)malloc(AES256_KEY_LEN);
    if (!used_key) return result;
    memcpy(used_key, key, AES256_KEY_LEN);

    // Generate IV
    unsigned char iv_local[AES_BLOCK_SIZE];
    if (RAND_bytes(iv_local, AES_BLOCK_SIZE) != 1) {
        free(used_key);
        return result;
    }
    unsigned char* iv_out = (unsigned char*)malloc(AES_BLOCK_SIZE);
    if (!iv_out) {
        free(used_key);
        return result;
    }
    memcpy(iv_out, iv_local, AES_BLOCK_SIZE);

    // Prepare AES key structure
    AES_KEY aes_key;
    if (AES_set_encrypt_key(used_key, AES256_KEY_LEN * 8, &aes_key) != 0) {
        free(used_key);
        free(iv_out);
        return result;
    }

    // Pad data
    size_t padded_len = 0;
    char* padded = pad_data(data, data_len, &padded_len);
    if (!padded) {
        free(used_key);
        free(iv_out);
        return result;
    }

    // Allocate output buffer
    unsigned char* out_buf = (unsigned char*)malloc(padded_len);
    if (!out_buf) {
        free(used_key);
        free(iv_out);
        free(padded);
        return result;
    }

    // AES_cbc_encrypt modifies the ivec, so provide a local copy
    unsigned char ivec[AES_BLOCK_SIZE];
    memcpy(ivec, iv_local, AES_BLOCK_SIZE);

    AES_cbc_encrypt((const unsigned char*)padded, out_buf, padded_len, &aes_key, ivec, AES_ENCRYPT);

    // Clean up padded buffer
    memset(padded, 0, padded_len);
    free(padded);

    // Fill result
    result.encrypted_data = out_buf;
    result.encrypted_len = padded_len;
    result.iv = iv_out;
    result.key = used_key;

    return result;
}
