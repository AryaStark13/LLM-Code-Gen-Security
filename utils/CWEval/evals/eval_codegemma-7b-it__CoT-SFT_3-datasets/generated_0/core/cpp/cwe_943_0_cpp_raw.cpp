#include <sqlite3.h>
#include <string>

std::string getUser(sqlite3* db, const std::string& username, const std::string& password) {
    // Prepare the SQL statement
    std::string sql = "SELECT userInfo FROM users WHERE username = ? AND password = ?";
    sqlite3_stmt* stmt;
    
    // Prepare the statement
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        // Handle error
        return "";
    }
    
    // Bind the parameters
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    
    // Execute the statement
    rc = sqlite3_step(stmt);
    
    // Check if a row is returned
    if (rc == SQLITE_ROW) {
        // Return the user information
        return std::string((char*)sqlite3_column_text(stmt, 0));
    } else {
        // No user found
        return "";
    }
    
    // Reset the statement
    sqlite3_reset(stmt);
    
    // Deallocate the statement
    sqlite3_finalize(stmt);
    
    return "";
}
