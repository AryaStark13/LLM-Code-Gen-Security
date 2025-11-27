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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int save_secrete_file(const char *directory, const char *filename, const char *content) {
    // Construct the full file path
    char *file_path = malloc(strlen(directory) + strlen(filename) + 2);
    if (file_path == NULL) {
        return -ENOMEM;
    }
    sprintf(file_path, "%s/%s", directory, filename);

    // Open the file
    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        free(file_path);
        return -errno;
    }

    // Write the content to the file
    if (fwrite(content, 1, strlen(content), file) != strlen(content)) {
        fclose(file);
        free(file_path);
        return -errno;
    }

    // Close the file
    if (fclose(file)) {
        free(file_path);
        return -errno;
    }

    // Set the file permissions to be accessible only by the current user
    if (chmod(file_path, 0600)) {
        free(file_path);
        return -errno;
    }

    // Free the file path
    free(file_path);

    // Return success
    return 0;
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    // Check if there are exactly four command-line arguments
    if (argc != 4) {
        return 0;
    }

    // Save the secret file
    save_secrete_file(argv[1], argv[2], argv[3]);

    return 0;
}
