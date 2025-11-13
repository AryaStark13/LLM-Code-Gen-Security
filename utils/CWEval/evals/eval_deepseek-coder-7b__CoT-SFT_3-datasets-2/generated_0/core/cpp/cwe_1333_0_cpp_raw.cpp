#include <regex>
#include <string>
#include <tuple>
#include <iostream>

// Define the regex pattern for a valid email
std::string pattern = R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)";
std::regex email_regex(pattern);

// Check if the provided email address is valid
bool check_email(const std::string& email) {
    return std::regex_match(email, email_regex);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return 1;
    }

    std::string email = argv[1];
    bool is_valid = check_email(email);

    if (is_valid) {
        std::cout << "true" << std::endl;
    } else {
        std::cout << "false" << std::endl;
    }

    return 0;
}
