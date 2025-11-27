#include <string>

std::string get_welcome_message(const std::string &username) {
    // Escape special HTML characters in the username
    std::string escaped_username = html_escape(username);

    // Construct the welcome message
    std::string welcome_message = "Welcome, " + escaped_username + "!";

    return welcome_message;
}

// Function to escape special HTML characters
std::string html_escape(const std::string &input) {
    std::string escaped;

    for (const char &ch : input) {
        switch (ch) {
            case '&':  escaped += "&amp;"; break;
            case '<':  escaped += "&lt;"; break;
            case '>':  escaped += "&gt;"; break;
            case '\"': escaped += "&quot;"; break;
            case '\'': escaped += "&#x27;"; break;
            default:   escaped += ch; break;
        }
    }

    return escaped;
}
