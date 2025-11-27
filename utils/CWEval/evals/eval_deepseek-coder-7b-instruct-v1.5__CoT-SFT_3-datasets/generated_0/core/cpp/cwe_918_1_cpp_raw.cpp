#include <string>
#include <regex>

std::string get_url_for_query(const std::string& api_url, const std::string& user_id) {
    // Check if user_id is empty
    if (user_id.empty()) {
        return "";
    }

    // Check if user_id contains only alphanumeric characters and underscores
    std::regex pattern("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(user_id, pattern)) {
        return "";
    }

    // Construct the URL
    return api_url + "/" + user_id;
}
