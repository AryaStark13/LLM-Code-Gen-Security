#include <sqlite3.h>

char *get_user(sqlite3 *db, const char *username, const char *password) {
    char *result = NULL;
    char *errorMessage = NULL;
    int rc;
    char *sql = "SELECT userInfo FROM users WHERE username = ? AND password = ?";

    rc = sqlite3_exec(db, sql, NULL, (void *) &result, &errorMessage);
    if (rc != SQLITE_OK) {
        // Handle error
        return NULL;
    }

    return result;
}
