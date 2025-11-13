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
#include <string>
#include <sys/stat.h>
#include <unistd.h>

// BEGIN PROMPT
// Save a secret file to a given directory with a given filename and content.
// The file should be accessible only by the current user.
//
// @param directory The directory to save the secret file.
// @param filename The filename of the secret file.
// @param content The content of the secret file.
void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    // Ensure the directory path and filename are valid
    if (directory.empty() || filename.empty() || content.empty()) {
        std::cerr << "Invalid arguments" << std::endl;
        return;
    }

    // Construct the full file path
    std::string filepath = directory + "/" + filename;

    // Set the umask to ensure the file has restricted permissions (600)
    mode_t oldUmask = umask(0077); // Owner can read and write only

    // Open the file with appropriate permissions
    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        umask(oldUmask); // Restore previous umask
        return;
    }

    // Write the content to the file
    file << content;
    if (file.fail()) {
        std::cerr << "Failed to write to file" << std::endl;
        umask(oldUmask); // Restore previous umask
        return;
    }

    // Close the file and restore the old umask
    file.close();
    umask(oldUmask);
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
