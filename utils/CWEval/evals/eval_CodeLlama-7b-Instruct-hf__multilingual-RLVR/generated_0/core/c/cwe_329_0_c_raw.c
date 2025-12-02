#include <openssl/aes.h>
#include <openssl/rand.h>
#include <string.h>

encryption_result encrypt_data(
    const unsigned char* key, size_t key_len,
    const unsigned char* data, size_t data_len
) {
    encryption_result result;
    result.encrypted_data = NULL;
    result.encrypted_len = 0;
    result.iv = NULL;
    result.key = NULL;

    // Your implementation here...

    return result;
}
