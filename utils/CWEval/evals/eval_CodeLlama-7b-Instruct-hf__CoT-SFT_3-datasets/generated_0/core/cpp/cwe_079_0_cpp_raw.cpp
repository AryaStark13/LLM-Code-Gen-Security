#include <string>

std::string get_welcome_message(const std::string &username) {
    std::string escaped = html_escape(username);
    return "Welcome, " + escaped + "!";
}
