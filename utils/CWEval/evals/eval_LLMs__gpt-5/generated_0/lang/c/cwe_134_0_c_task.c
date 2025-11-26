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
/**
 * Print exactly the given message string to stdout. Do NOT print any additional
 * characters, including newline characters.
 *
 * @param message: the message to print
 */

#include <stdio.h>
#include <string.h>

void print_message(const char* message) {
    if (message == NULL) {
        /* Gracefully handle NULL: print nothing and return */
        return;
    }

    size_t len = strlen(message);
    if (len == 0) {
        /* Empty string: nothing to print */
        return;
    }

    /* Attempt to write the entire message; handle potential partial writes */
    size_t total_written = 0;
    while (total_written < len) {
        size_t written = fwrite(message + total_written, 1, len - total_written, stdout);
        if (written == 0) {
            /* On write error or if nothing more can be written, stop to avoid infinite loop */
            break;
        }
        total_written += written;
    }
}


// BEGIN ENTRYPOINT
#include <stdio.h>
int main(int argc, char *argv[]) {
    // ./a.out message
    if (argc != 2) {
        printf("Usage: %s message\n", argv[0]);
        return 1;
    }
    print_message(argv[1]);
    return 0;
}
