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
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

// Save a secret file to a given directory with a given filename and content.
// The file should be accessible only by the current user.
//
// @param directory The directory to save the secret file.
// @param filename The filename of the secret file.
// @param content The content of the secret file.
void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    // Check if the directory exists and is accessible by the current user
    struct stat st;
    if (stat(directory.c_str(), &st) == -1) {
        std::cerr << "Error: Directory " << directory << " does not exist." << std::endl;
        return;
    }
    if (st.st_uid != getuid()) {
        std::cerr << "Error: Directory " << directory << " is not accessible by the current user." << std::endl;
        return;
    }

    // Check if the file already exists and is owned by the current user
    std::string filePath = directory + "/" + filename;
    if (access(filePath.c_str(), F_OK) == 0) {
        if (stat(filePath.c_str(), &st) == -1) {
            std::cerr << "Error: Failed to access file " << filePath << std::endl;
            return;
        }
        if (st.st_uid != getuid()) {
            std::cerr << "Error: File " << filePath << " is owned by another user." << std::endl;
            return;
        }
    }

    // Write the content to the file
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << filePath << std::endl;
        return;
    }
    file << content;
    file.close();

    // Set the file permissions to 600 (read/write for owner only)
    if (chmod(filePath.c_str(), 0600) == -1) {
        std::cerr << "Error: Failed to set permissions for file " << filePath << std::endl;
        return;
    }
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    // Check if there are exactly four command-line arguments
    if (argc != 4) {
        return 1;
    }

    // Save the secret file
    saveSecreteFile(argv[1], argv[2], argv[3]);

    return 0;
}
