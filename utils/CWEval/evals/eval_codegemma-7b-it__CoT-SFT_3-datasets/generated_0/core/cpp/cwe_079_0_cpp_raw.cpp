#include <string>

std::string get_welcome_message(const std::string &username) {
    if (username.empty()) {
        return "Welcome, guest!";
    } else {
        return "Welcome, " + username + "!";
    }
}
