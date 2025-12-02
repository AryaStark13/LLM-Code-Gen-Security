#include <string>

std::string getUser(sqlite3* db, const std::string& username, const std::string& password) {
    std::string query = "SELECT userInfo FROM users WHERE username=? AND password=?";
    char* errMsg;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        return "";
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* userInfo = sqlite3_column_text(stmt, 0);
        return userInfo ? std::string(reinterpret_cast<const char*>(userInfo)) : "";
    }
    sqlite3_finalize(stmt);
    return "";
}
