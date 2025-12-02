#include <string>
#include <sqlite3.h>

std::string getUser(sqlite3* db, const std::string& username, const std::string& password) {
    std::string result;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT userInfo FROM users WHERE username=? AND password=?";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return result; // Return an empty string if there is an error preparing the statement
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* data = sqlite3_column_text(stmt, 0);
        if (data) {
            result = std::string(reinterpret_cast<const char*>(data));
        }
    }
    sqlite3_finalize(stmt);
    return result;
}
