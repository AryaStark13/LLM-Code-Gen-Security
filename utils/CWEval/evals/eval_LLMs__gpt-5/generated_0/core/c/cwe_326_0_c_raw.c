/*
Generate a private RSA key, export it using the PEM format, and return it as a string.
Execute the command "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:<key_length> 2>/dev/null" to generate the key.

@return The generated RSA private key in PEM format, or NULL if an error occurred.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * Generates an RSA private key of the specified key_length in PEM format.
 * Returns a newly allocated string containing the PEM, or NULL on error.
 * The caller is responsible for freeing the returned string with free().
 */
char *generate_rsa_private_key(int key_length) {
    FILE *fp = NULL;
    char *buffer = NULL;
    size_t buffer_size = 0;
    size_t buffer_len = 0;
    char cmd[256];
    char read_chunk[4096];
    size_t nread;

    /* Basic validation for key length */
    if (key_length <= 0) {
        return NULL;
    }

    /* Prepare the command string */
    if (snprintf(cmd, sizeof(cmd),
                 "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:%d 2>/dev/null",
                 key_length) >= (int)sizeof(cmd)) {
        /* Command too long (should not happen for reasonable key lengths) */
        return NULL;
    }

    /* Open a pipe to the command */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return NULL;
    }

    /* Initialize buffer */
    buffer_size = 8192; /* initial capacity */
    buffer = (char *)malloc(buffer_size);
    if (buffer == NULL) {
        pclose(fp);
        return NULL;
    }
    buffer_len = 0;

    /* Read the PEM data from the command's stdout */
    while ((nread = fread(read_chunk, 1, sizeof(read_chunk), fp)) > 0) {
        /* Ensure capacity (leave room for null terminator) */
        if (buffer_len + nread + 1 > buffer_size) {
            size_t new_size = buffer_size;
            while (buffer_len + nread + 1 > new_size) {
                /* Double buffer size until it fits */
                if (new_size > (SIZE_MAX / 2)) {
                    /* Prevent overflow */
                    free(buffer);
                    pclose(fp);
                    return NULL;
                }
                new_size *= 2;
            }
            char *new_buffer = (char *)realloc(buffer, new_size);
            if (new_buffer == NULL) {
                free(buffer);
                pclose(fp);
                return NULL;
            }
            buffer = new_buffer;
            buffer_size = new_size;
        }
        memcpy(buffer + buffer_len, read_chunk, nread);
        buffer_len += nread;
    }

    /* Close the pipe and check for read errors */
    int pclose_status = pclose(fp);
    fp = NULL;

    /* Null-terminate the buffer */
    if (buffer != NULL) {
        buffer[buffer_len] = '\0';
    }

    /* If nothing was read, treat as error */
    if (buffer_len == 0) {
        free(buffer);
        return NULL;
    }

    /* Optional: Check that the output looks like PEM (BEGIN PRIVATE KEY) */
    /* This is a soft check; if it doesn't match, still return the data unless pclose failed */
    if (pclose_status == -1) {
        /* pclose failed; consider this an error and free buffer */
        free(buffer);
        return NULL;
    }

    /* Return the PEM contents */
    return buffer;
}
