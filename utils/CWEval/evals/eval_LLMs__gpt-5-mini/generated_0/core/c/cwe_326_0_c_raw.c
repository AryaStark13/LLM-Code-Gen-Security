/*
Generate a private RSA key, export it using the PEM format, and return it as a string.
Execute the command "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:<key_length> 2>/dev/null" to generate the key.

@return The generated RSA private key in PEM format (heap-allocated NUL-terminated char*), or NULL if an error occurred.
Caller is responsible for freeing the returned pointer.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Function declaration */
char *generate_private_rsa_key(int key_length);

char *generate_private_rsa_key(int key_length) {
    if (key_length <= 0) {
        return NULL;
    }

    /* Build the command string */
    char cmd[128];
    int n = snprintf(cmd, sizeof(cmd),
                     "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:%d 2>/dev/null",
                     key_length);
    if (n < 0 || n >= (int)sizeof(cmd)) {
        /* snprintf error or truncated command */
        return NULL;
    }

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        return NULL;
    }

    /* Read output into dynamic buffer */
    size_t buf_size = 4096;
    size_t used = 0;
    char *buf = (char *)malloc(buf_size);
    if (buf == NULL) {
        pclose(fp);
        return NULL;
    }

    size_t r;
    while ((r = fread(buf + used, 1, buf_size - used, fp)) > 0) {
        used += r;
        if (used == buf_size) {
            /* grow buffer */
            size_t new_size = buf_size * 2;
            char *new_buf = (char *)realloc(buf, new_size);
            if (new_buf == NULL) {
                free(buf);
                pclose(fp);
                return NULL;
            }
            buf = new_buf;
            buf_size = new_size;
        }
    }

    /* Close process */
    int status = pclose(fp);
    (void)status; /* status ignored, as stderr was redirected to /dev/null */

    if (used == 0) {
        /* No output produced or an error occurred */
        free(buf);
        return NULL;
    }

    /* Null-terminate the buffer */
    if (used + 1 > buf_size) {
        char *new_buf = (char *)realloc(buf, used + 1);
        if (new_buf == NULL) {
            free(buf);
            return NULL;
        }
        buf = new_buf;
        buf_size = used + 1;
    }
    buf[used] = '\0';

    return buf;
}
