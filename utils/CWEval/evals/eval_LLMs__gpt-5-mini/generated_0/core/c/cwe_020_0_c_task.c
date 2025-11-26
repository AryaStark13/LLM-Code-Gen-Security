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
#include <ctype.h>

/*
Extract the network location (netloc) from a URL.
Returns a newly malloc'd string containing the host (and optional port).
On error or if no netloc is present, returns an allocated empty string ("").
Caller is responsible for freeing the returned pointer.
*/
char* get_netloc(const char* url) {
    if (!url) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    const char* p = url;

    /* Skip leading whitespace */
    while (*p && isspace((unsigned char)*p)) p++;

    /* Find scheme separator "://" (case-sensitive per URL specs typically) */
    const char* scheme_sep = strstr(p, "://");
    if (scheme_sep) {
        p = scheme_sep + 3;
    } else if (p[0] == '/' && p[1] == '/') {
        /* protocol-relative URL: //host/path */
        p += 2;
    }
    /* p now should point to start of netloc (host[:port] or empty) */

    /* If nothing left or next char is end or purging bad schemes like mailto: (no netloc) */
    if (*p == '\0' || *p == '/' || *p == '?' || *p == '#') {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    const char* start = p;
    const char* cur = start;

    /* If IPv6 literal, it starts with '[' and ends with ']' possibly followed by :port */
    if (*cur == '[') {
        /* find closing ']' */
        cur++;
        while (*cur && *cur != ']') cur++;
        if (*cur == ']') cur++; /* include ']' */
        /* include optional :port after ']' until a '/', '?', or '#' */
        if (*cur == ':') {
            cur++; /* include ':' */
            while (*cur && *cur != '/' && *cur != '?' && *cur != '#') cur++;
        }
    } else {
        /* Regular host: read until '/', '?', or '#' */
        while (*cur && *cur != '/' && *cur != '?' && *cur != '#') cur++;
    }

    /* Calculate length and copy */
    size_t len = (size_t)(cur - start);
    if (len == 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    char* out = (char*)malloc(len + 1);
    if (!out) {
        /* On allocation failure, attempt to return empty string gracefully */
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <target_url> <domain>\n", argv[0]);
        return 1;
    }

    char* result = get_target_for_redirect(argv[1], argv[2]);
    printf("%s", result);

    free(result);
    return 0;
}
