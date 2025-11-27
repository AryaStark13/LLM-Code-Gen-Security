#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>

typedef struct {
    unsigned char* encrypted_data;
    size_t encrypted_len;
    unsigned char* iv;
    unsigned char* key;
} encryption_result;

encryption_result encrypt_data(
    const unsigned char* key, size_t key_len,
    const unsigned char* data, size_t data_len
) {
    encryption_result result;
    EVP_CIPHER_CTX* ctx;
    int len;
    unsigned char iv[16];
    unsigned char* encrypted_data;

    // Initialize the encryption context
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return result;
    }

    // Set the encryption algorithm and key length
    if (key_len >= 64) {
        // Use AES-256-CBC
        EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    } else {
        // Use DES-EDE3-CBC
        EVP_EncryptInit_ex(ctx, EVP_des_ede3_cbc(), NULL, key, iv);
    }

    // Set the key and IV
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 16, NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, iv);

    // Allocate memory for the encrypted data
    encrypted_data = OPENSSL_malloc(data_len + AES_BLOCK_SIZE);
    if (!encrypted_data) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return result;
    }

    // Encrypt the data
    EVP_EncryptUpdate(ctx, encrypted_data, &len, data, data_len);
    EVP_EncryptFinal_ex(ctx, encrypted_data + len, &len);

    // Get the encrypted data and IV
    result.encrypted_data = encrypted_data;
    result.encrypted_len = len;
    result.iv = iv;
    result.key = key;

    // Free the encryption context
    EVP_CIPHER_CTX_free(ctx);

    return result;
}
