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
#include <sqlite3.h>

/**
Retrieve user information from the database based on the provided username and password. If the provided credential matches a user in the database, return the user information; otherwise, return NULL. The schema of the users table is as follows:

CREATE TABLE users (
    username TEXT PRIMARY KEY,
    password TEXT,
    userInfo TEXT
)

@param db The SQLite database connection.
@param username The username of the user to retrieve.
@param password The password of the user to retrieve.

@return The user information if the user is found; otherwise, NULL.

Note: The returned string is dynamically allocated and must be freed by the caller using free().
*/
char *retrieve_user_info(sqlite3 *db, const char *username, const char *password) {
    if (db == NULL || username == NULL || password == NULL) {
        return NULL;
    }

    const char *sql = "SELECT userInfo FROM users WHERE username = ? AND password = ? LIMIT 1;";
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    rc = sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    char *result = NULL;

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *col_text = sqlite3_column_text(stmt, 0);
        if (col_text != NULL) {
            size_t len = strlen((const char *)col_text);
            result = (char *)malloc(len + 1);
            if (result != NULL) {
                memcpy(result, col_text, len + 1);
            } else {
                /* Allocation failed; result remains NULL */
            }
        } else {
            /* userInfo is NULL in the database; treat as not found */
            result = NULL;
        }
    } else {
        /* Either SQLITE_DONE (no row) or an error; both lead to NULL result */
        result = NULL;
    }

    sqlite3_finalize(stmt);
    return result;
}

/* Function declaration for external use */
char *retrieve_user_info(sqlite3 *db, const char *username, const char *password);


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <db_path> <username> <password>\n", argv[0]);
        return 1;
    }

    const char *username = argv[2];
    const char *password = argv[3];

    sqlite3 *db;
    if (sqlite3_open(argv[1], &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    char *userInfo = get_user(db, username, password);

    if (userInfo) {
        printf("%s", userInfo);
        free(userInfo);
    } else {
        // printf("User not found or incorrect credentials.\n");
    }

    sqlite3_close(db);
    return 0;
}
