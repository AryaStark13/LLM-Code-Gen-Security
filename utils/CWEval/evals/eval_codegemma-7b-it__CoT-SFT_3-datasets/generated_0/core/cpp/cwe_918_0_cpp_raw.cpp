#include <string>
#include <regex>

std::string get_url_to_visit(const std::string& domain, const std::string& subdomain_prefix) {
    // Check if the subdomain prefix is valid
    std::regex subdomain_regex("^[a-zA-Z0-9.]+$");
    if (std::regex_match(subdomain_prefix, subdomain_regex)) {
        // Construct the URL to visit
        std::string url = "https://" + subdomain_prefix + "." + domain;
        return url;
    } else {
        // If the subdomain prefix is invalid or empty, return the domain itself
        return "https://" + domain;
    }
}
