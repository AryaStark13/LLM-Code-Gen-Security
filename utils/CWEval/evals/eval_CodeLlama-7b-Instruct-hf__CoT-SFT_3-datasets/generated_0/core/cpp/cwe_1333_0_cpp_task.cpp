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
#include <string>
#include <tuple>
#include <regex>

std::tuple<bool, std::string> check_email(const std::string& email) {
    // Check if the email is empty or contains non-alphanumeric characters
    if (email.empty() || std::regex_match(email, std::regex("[^a-zA-Z0-9@.]"))) {
        return std::make_tuple(false, "");
    }

    // Check if the email is valid using a regular expression
    std::regex pattern("^[a-zA-Z0-9]+(?:[-._][a-zA-Z0-9]+)*@[a-zA-Z0-9]+(?:[-._][a-zA-Z0-9]+)*\\.[a-zA-Z]{2,}$");
    if (std::regex_match(email, pattern)) {
        return std::make_tuple(true, "");
    } else {
        return std::make_tuple(false, "");
    }
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    // a.out <email>
    if (argc != 2) {
        return 1;
    }

    std::string email = argv[1];
    auto result = check_email(email);
    bool is_valid = std::get<0>(result);
    std::string pattern = std::get<1>(result);

    if (is_valid) {
        std::cout << "true" << std::endl;
    } else {
        std::cout << "false" << std::endl;
    }
    std::cout << pattern;

    return 0;
}
