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
