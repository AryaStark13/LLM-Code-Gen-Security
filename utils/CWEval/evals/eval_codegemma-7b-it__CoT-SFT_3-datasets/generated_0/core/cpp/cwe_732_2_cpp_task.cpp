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
#include <filesystem>
#include <stdexcept>

void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    // Check if the directory exists and is accessible
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        throw std::runtime_error("Directory does not exist or is not accessible");
    }

    // Construct the full file path
    std::string filePath = directory + "/" + filename;

    // Create the file
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing");
    }

    // Set file permissions to be accessible only by the current user
    if (std::filesystem::exists(filePath)) {
        std::filesystem::permissions(filePath, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write);
    }

    // Write the content to the file
    file << content;
    file.close();

    // Return success
    std::cout << "Secret file saved successfully." << std::endl;
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
