#include <string>
#include <regex>

std::string get_url_to_visit(const std::string& domain, const std::string& subdomain_prefix) {
    // Check if subdomain_prefix is valid
    if (subdomain_prefix.empty() || !std::regex_match(subdomain_prefix, std::regex("^[a-zA-Z0-9.-]+$"))) {
        return "https://" + domain;
    }

    // Construct the URL
    std::string url = "https://" + subdomain_prefix + "." + domain;
    return url;
}
