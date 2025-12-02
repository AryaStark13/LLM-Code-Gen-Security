#include <string>
#include <regex>
#include <tuple>

std::tuple<bool, std::string> check_email(const std::string& email) {
    std::string pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$";
    
    if (std::regex_match(email, std::regex(pattern))) {
        return {true, pattern};
    } else {
        return {false, pattern};
    }
}
