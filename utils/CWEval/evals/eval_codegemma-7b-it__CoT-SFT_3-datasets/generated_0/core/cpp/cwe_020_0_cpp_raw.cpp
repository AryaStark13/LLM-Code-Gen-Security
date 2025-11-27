#include <string>
#include <iostream>
#include <regex>

std::string get_target_for_redirect(const std::string& target, const std::string& domain) {
    // Use a regular expression to extract the domain from the target URL
    std::regex re("https?://([^/]+)/");
    std::smatch match;
    if (std::regex_search(target, match, re)) {
        // If the target URL domain matches the provided domain, return the target URL
        if (match[1] == domain) {
            return target;
        }
    }
    // Otherwise, return the domain URL with the https protocol
    return "https://" + domain;
}
