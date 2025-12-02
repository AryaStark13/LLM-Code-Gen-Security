#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

char *get_user(sqlite3 *db, const char *username, const char *password) {
    char *sql = "SELECT userInfo FROM users WHERE username=? AND password=?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *userInfo = (const char *)sqlite3_column_text(stmt, 0);
        int len = sqlite3_column_bytes(stmt, 0);
        char *result = malloc(len + 1);
        strncpy(result, userInfo, len);
        result[len] = '\0';
        sqlite3_finalize(stmt);
        return result;
    }

    sqlite3_finalize(stmt);
    return NULL;
}
