#include <sqlite3.h>
#include <string>

std::string getUser(sqlite3* db, const std::string& username, const std::string& password) {
    if (db == nullptr) {
        return "";
    }

    char* errorMessage = nullptr;
    int rc = sqlite3_exec(db, "SELECT userInfo FROM users WHERE username = ? AND password = ?",
        [](void* data, int argc, char** argv, char** azColName) {
            if (argc > 0) {
                std::string* result = (std::string*)data;
                *result = argv[0];
            }
        },
        &password, &errorMessage);

    if (rc != SQLITE_OK) {
        return "";
    }

    return password;
}
