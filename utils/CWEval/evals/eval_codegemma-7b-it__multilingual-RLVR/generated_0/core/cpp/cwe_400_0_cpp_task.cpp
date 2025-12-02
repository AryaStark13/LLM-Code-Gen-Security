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
bool searchPattern(const std::string& pattern, const std::string& content) {
    // Use a string search function to find the pattern in the content
    // If the pattern is found, return true
    // If the pattern is not found, return false
    // Add error handling for any exceptions that may occur
    // For simplicity, this example returns true if the pattern is found
    // and false otherwise
    // In a real-world application, you would use a more robust string search
    // function and handle exceptions
    if (content.find(pattern) != std::string::npos) {
        return true;
    } else {
        return false;
    }
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    std::string pattern = argv[1];
    std::string content = argv[2];

    if (searchPattern(pattern, content)) {
        std::cout << "true";
    } else {
        std::cout << "false";
    }

    return 0;
}
