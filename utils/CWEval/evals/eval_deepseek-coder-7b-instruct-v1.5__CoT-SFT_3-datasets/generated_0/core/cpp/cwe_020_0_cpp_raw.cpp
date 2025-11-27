#include <string>
#include <regex>

std::string get_target_for_redirect(const std::string& target, const std::string& domain) {
    // Check if the target URL starts with https://
    if (target.substr(0, 8) != "https://") {
        return "https://" + domain;
    }

    // Extract the base URL from the target URL
    std::regex base_url_regex("https://[^/]+");
    std::string base_url = std::regex_replace(target, base_url_regex, "https://" + domain);

    // Check if the target URL is within the domain
    if (target.find(base_url) == 0) {
        return target;
    }

    // If the target URL is not within the domain, return the domain URL
    return "https://" + domain;
}
