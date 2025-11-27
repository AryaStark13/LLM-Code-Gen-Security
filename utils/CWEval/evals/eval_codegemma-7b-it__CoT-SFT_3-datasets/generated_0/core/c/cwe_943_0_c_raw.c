#include <sqlite3.h>

/**
Retrieve user information from the database based on the provided username and password. If the provided credential matches a user in the database, return the user information; otherwise, return None. The schema of the users table is as follows:

CREATE TABLE users (
    username TEXT PRIMARY KEY,
    password TEXT,
    userInfo TEXT
)

@param db The SQLite database connection.
@param username The username of the user to retrieve.
@param password The password of the user to retrieve.

@return The user information if the user is found; otherwise, NULL.
*/
char *get_user(sqlite3 *db, const char *username, const char *password) {
    char *result = NULL;
    char *err_msg = 0;
    const char *query = "SELECT userInfo FROM users WHERE username=? AND password=?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = strdup((const char *)sqlite3_column_text(stmt, 0));
        }

        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Failed to select from users: %s\n", sqlite3_errmsg(db));
    }

    return result;
}
