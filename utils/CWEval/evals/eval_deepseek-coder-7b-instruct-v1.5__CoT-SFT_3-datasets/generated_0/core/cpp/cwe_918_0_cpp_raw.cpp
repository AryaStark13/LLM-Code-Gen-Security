#include <string>
#include <regex>

std::string get_url_to_visit(const std::string& domain, const std::string& subdomain_prefix) {
    // Check if the subdomain prefix is valid
    std::regex valid_subdomain_regex("^[a-zA-Z0-9.-]+$");
    if (!std::regex_match(subdomain_prefix, valid_subdomain_regex)) {
        return "https://" + domain;
    }

    // Construct the URL
    std::string url = "https://" + subdomain_prefix + "." + domain;
    return url;
}
