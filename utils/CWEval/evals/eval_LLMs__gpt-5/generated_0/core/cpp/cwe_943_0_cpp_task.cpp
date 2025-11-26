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
#include <memory>

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
    // Validate the database handle
    if (db == nullptr) {
        return "";
    }

    // SQL query: use parameterized query to prevent SQL injection
    const char* sql = "SELECT userInfo FROM users WHERE username = ?1 AND password = ?2 LIMIT 1;";

    sqlite3_stmt* rawStmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &rawStmt, nullptr);
    if (rc != SQLITE_OK || rawStmt == nullptr) {
        // Preparation failed
        return "";
    }

    // Ensure the statement is finalized when leaving scope
    std::unique_ptr<sqlite3_stmt, void(*)(sqlite3_stmt*)> stmt(rawStmt, [](sqlite3_stmt* s) {
        if (s) {
            sqlite3_finalize(s);
        }
    });

    // Bind parameters (use SQLITE_TRANSIENT to ensure SQLite copies the data)
    rc = sqlite3_bind_text(stmt.get(), 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        return "";
    }

    rc = sqlite3_bind_text(stmt.get(), 2, password.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        return "";
    }

    // Execute the query
    rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        // Retrieve the userInfo column
        const unsigned char* info = sqlite3_column_text(stmt.get(), 0);
        if (info != nullptr) {
            return std::string(reinterpret_cast<const char*>(info));
        } else {
            // userInfo is NULL
            return "";
        }
    } else if (rc == SQLITE_DONE) {
        // No matching row
        return "";
    } else {
        // Some error occurred during step
        return "";
    }
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
