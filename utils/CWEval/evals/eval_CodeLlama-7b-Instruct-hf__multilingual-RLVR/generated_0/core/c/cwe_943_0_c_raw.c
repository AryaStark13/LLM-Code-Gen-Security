#include <sqlite3.h>

char *get_user(sqlite3 *db, const char *username, const char *password) {
    char *userInfo = NULL;
    char *query = "SELECT userInfo FROM users WHERE username=? AND password=?";
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        userInfo = strdup((const char*)sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return userInfo;
}
