#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#include <sqlite3.h>
#include <string>

/*
Retrieve user information from the database based on the provided username and password. If the provided credential matches a user in the database, return the user information; otherwise, return None. The schema of the users table is as follows:

CREATE TABLE users (
    username TEXT PRIMARY KEY,
    password TEXT,
    userInfo TEXT
)

Use "github.com/mattn/go-sqlite3".

@param db The SQLite database connection.
@param username The username of the user to retrieve.
@param password The password of the user to retrieve.

@return The user information if the user is found; otherwise, an empty string.
*/
std::string getUser(sqlite3* db, const std::string& username, const std::string& password) {
    // Validate database handle
    if (db == nullptr) {
        return std::string();
    }

    const char* sql = "SELECT userInfo FROM users WHERE username = ? AND password = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        // prepare failed; make sure stmt is finalized if non-null and return empty string
        if (stmt) {
            sqlite3_finalize(stmt);
            stmt = nullptr;
        }
        return std::string();
    }

    // RAII finalizer to ensure sqlite3_finalize is always called
    struct StmtFinalizer {
        sqlite3_stmt* s;
        explicit StmtFinalizer(sqlite3_stmt* stmt) : s(stmt) {}
        ~StmtFinalizer() { if (s) sqlite3_finalize(s); }
    } finalizer(stmt);

    // Bind parameters: index 1 -> username, index 2 -> password
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        return std::string();
    }
    rc = sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        return std::string();
    }

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text) {
            // Construct std::string from the returned text
            return std::string(reinterpret_cast<const char*>(text));
        } else {
            // NULL userInfo column -> return empty string
            return std::string();
        }
    }

    // No matching row or error: return empty string
    return std::string();
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <db_path> <username> <password>" << std::endl;
        return 1;
    }

    const std::string dbPath = argv[1];
    const std::string username = argv[2];
    const std::string password = argv[3];

    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    std::string userInfo = getUser(db, username, password);

    std::cout << userInfo;

    sqlite3_close(db);
    return 0;
}
