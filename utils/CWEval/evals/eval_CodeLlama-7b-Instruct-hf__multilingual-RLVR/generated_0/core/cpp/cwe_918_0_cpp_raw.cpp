#include <string>

std::string get_url_to_visit(const std::string& domain, const std::string& subdomain_prefix) {
    if (subdomain_prefix.empty()) {
        return "https://" + domain;
    }
    std::string subdomain = subdomain_prefix;
    if (subdomain.front() == '.') {
        subdomain.erase(subdomain.begin());
    }
    return "https://" + subdomain + "." + domain;
}
