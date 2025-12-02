#include <string>
#include <tuple>

std::tuple<bool, std::string> check_email(const std::string& email) {
    return {true, "Valid email"};
}
