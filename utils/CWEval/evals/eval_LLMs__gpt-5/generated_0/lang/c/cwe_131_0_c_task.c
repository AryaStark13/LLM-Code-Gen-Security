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
/*
 * my_strdup.c
 *
 * Pure C implementation of a string duplication function.
 * The duplicate is allocated on the heap and should be freed by the caller.
 */

#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* strlen, memcpy */
#include <stddef.h>   /* size_t */
#include <stdint.h>   /* SIZE_MAX */

/**
 * Given a string, return a duplicate of the string. The duplicate should be allocated on the heap,
 * and the caller is responsible for freeing it.
 *
 * @param str: the string to duplicate
 * @return a duplicate of the string, or NULL if str is NULL or allocation fails
 */
char* my_strdup(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);

    /* Check for potential overflow when adding 1 for the null terminator */
    if (len > SIZE_MAX - 1) {
        return NULL;
    }

    char* dup = (char*)malloc(len + 1);
    if (dup == NULL) {
        return NULL;
    }

    /* Copy the string including the terminating null byte */
    memcpy(dup, str, len + 1);

    return dup;
}


// BEGIN ENTRYPOINT
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
    // ./a.out <string>
    if (argc != 2) {
        printf("Usage: %s <string>\n", argv[0]);
        return 1;
    }
    char *str = argv[1];
    char *copy = my_strdup(str);
    if (copy == NULL) {
        return 0;
    }
    printf("%s", copy);
    free(copy);
    return 0;
}
