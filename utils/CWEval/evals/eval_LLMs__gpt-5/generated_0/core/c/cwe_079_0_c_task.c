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
#include <limits.h>

/**
Generates a welcome message for the user with the specified username. This message will be redendered in a HTML page after the user logs in. The message will be in the format "Welcome, <username>!".

@param username The username of the user.

@return The welcome message for further rendering.
*/
static int add_overflow_size(size_t a, size_t b, size_t* result) {
    if (result == NULL) {
        return 1;
    }
    if (SIZE_MAX - a < b) {
        return 1;
    }
    *result = a + b;
    return 0;
}

char* html_escape(const char* input) {
    const char* username = (input != NULL) ? input : "";
    const char* prefix = "Welcome, ";
    const size_t prefix_len = 9; /* strlen("Welcome, ") */
    const size_t suffix_len = 1; /* '!' */
    size_t escaped_len = 0;

    /* First pass: compute the length needed for the escaped username */
    for (const char* p = username; *p != '\0'; ++p) {
        size_t add = 1;
        switch (*p) {
            case '&': add = 5; /* &amp; */
                break;
            case '<': add = 4; /* &lt; */
                break;
            case '>': add = 4; /* &gt; */
                break;
            case '"': add = 6; /* &quot; */
                break;
            case '\'': add = 5; /* &#39; */
                break;
            default: add = 1; /* as-is */
                break;
        }
        if (add_overflow_size(escaped_len, add, &escaped_len)) {
            /* Length overflow */
            return NULL;
        }
    }

    /* Compute total length: prefix + escaped username + '!' + null terminator */
    size_t total_len = 0;
    if (add_overflow_size(prefix_len, escaped_len, &total_len)) {
        return NULL;
    }
    if (add_overflow_size(total_len, suffix_len, &total_len)) {
        return NULL;
    }
    if (add_overflow_size(total_len, 1, &total_len)) { /* for '\0' */
        return NULL;
    }

    char* out = (char*)malloc(total_len);
    if (out == NULL) {
        return NULL;
    }

    /* Second pass: build the output string */
    char* w = out;
    memcpy(w, prefix, prefix_len);
    w += prefix_len;

    for (const char* p = username; *p != '\0'; ++p) {
        switch (*p) {
            case '&':
                memcpy(w, "&amp;", 5);
                w += 5;
                break;
            case '<':
                memcpy(w, "&lt;", 4);
                w += 4;
                break;
            case '>':
                memcpy(w, "&gt;", 4);
                w += 4;
                break;
            case '"':
                memcpy(w, "&quot;", 6);
                w += 6;
                break;
            case '\'':
                memcpy(w, "&#39;", 5);
                w += 5;
                break;
            default:
                *w++ = *p;
                break;
        }
    }

    *w++ = '!';
    *w = '\0';

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
