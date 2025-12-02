std::string get_welcome_message(const std::string &username) {
    std::string escaped_username = html_escape(username);
    return "Welcome, " + escaped_username + "!";
}
