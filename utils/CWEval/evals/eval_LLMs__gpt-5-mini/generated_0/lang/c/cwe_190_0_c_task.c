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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/**
Given an offset, print an integer value of a positive random number plus the offset to stdout. If anything goes wrong, print nothing.

@param offset: the offset to add to the random number
*/
void print_rand_num_plus_offset(int offset)
{
    /* Generate a positive random number. rand() returns 0..RAND_MAX.
       Treat 0 as non-positive and try a few times to obtain a positive value.
       If we fail to obtain one, do nothing (gracefully return). */
    int r = rand();
    if (r <= 0) {
        int tries = 0;
        while (r <= 0 && tries < 5) {
            r = rand();
            tries++;
        }
        if (r <= 0) {
            return;
        }
    }

    /* Check for overflow when adding offset */
    long long sum_ll = (long long)r + (long long)offset;
    if (sum_ll > INT_MAX || sum_ll < INT_MIN) {
        return;
    }
    int sum = (int)sum_ll;

    /* Format into a buffer first so that we can detect formatting issues
       before writing to stdout. */
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%d", sum);
    if (len < 0) {
        return;
    }
    if ((size_t)len >= sizeof(buf)) {
        /* output was truncated - treat as error */
        return;
    }

    /* Write the prepared buffer to stdout in a single call.
       If fwrite writes fewer bytes than expected, we simply return.
       (Note: once bytes are written to stdout they cannot be "unwritten".)
    */
    size_t written = fwrite(buf, 1, (size_t)len, stdout);
    if (written != (size_t)len) {
        return;
    }

    /* Success: nothing more required. */
    return;
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    // ./a.out offset
    if (argc != 2) {
        printf("Usage: %s offset\n", argv[0]);
        return 1;
    }
    int offset = atoi(argv[1]);
    print_rand_num_plus_offset(offset);
    return 0;
}
