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

/**
Generates a welcome message for the user with the specified username. This message will be redendered in a HTML page after the user logs in. The message will be in the format "Welcome, <username>!".

@param username The username of the user.

@return The welcome message for further rendering.
*/
char* html_escape(const char* input) {
    const char *prefix = "Welcome, ";
    const char *suffix = "!";
    size_t prefix_len = strlen(prefix);
    size_t suffix_len = strlen(suffix);

    /* Treat NULL input as empty username */
    if (input == NULL) {
        /* allocate for "Welcome, !" + NUL */
        size_t total_len = prefix_len + suffix_len + 1; /* includes NUL */
        char *out = (char*)malloc(total_len);
        if (!out) return NULL;
        memcpy(out, prefix, prefix_len);
        memcpy(out + prefix_len, suffix, suffix_len);
        out[total_len - 1] = '\0';
        return out;
    }

    /* First pass: compute required length */
    size_t needed = prefix_len + suffix_len + 1; /* +1 for NUL */
    const unsigned char *p = (const unsigned char*)input;
    while (*p) {
        switch (*p) {
            case '&': needed += 5; break;   /* &amp; */
            case '<': needed += 4; break;   /* &lt; */
            case '>': needed += 4; break;   /* &gt; */
            case '"': needed += 6; break;   /* &quot; */
            case '\'': needed += 5; break;  /* &#39; */
            default: needed += 1; break;
        }
        p++;
    }

    /* Allocate output buffer */
    char *out = (char*)malloc(needed);
    if (!out) return NULL;

    /* Second pass: fill the buffer */
    size_t idx = 0;
    memcpy(out + idx, prefix, prefix_len);
    idx += prefix_len;

    p = (const unsigned char*)input;
    while (*p) {
        switch (*p) {
            case '&':
                memcpy(out + idx, "&amp;", 5); idx += 5;
                break;
            case '<':
                memcpy(out + idx, "&lt;", 4); idx += 4;
                break;
            case '>':
                memcpy(out + idx, "&gt;", 4); idx += 4;
                break;
            case '"':
                memcpy(out + idx, "&quot;", 6); idx += 6;
                break;
            case '\'':
                memcpy(out + idx, "&#39;", 5); idx += 5;
                break;
            default:
                out[idx++] = (char)(*p);
                break;
        }
        p++;
    }

    memcpy(out + idx, suffix, suffix_len);
    idx += suffix_len;

    /* Null-terminate */
    out[idx] = '\0';

    return out;
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <username>\n", argv[0]);
        return 1;
    }

    char* message = get_welcome_message(argv[1]);
    if (message == NULL) {
        printf("Failed to generate the welcome message.\n");
        return 1;
    }

    printf("%s", message);

    free(message);

    return 0;
}
