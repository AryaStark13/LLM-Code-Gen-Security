#include <string>

std::string get_url_to_visit(const std::string& domain, const std::string& subdomain_prefix) {
    if (domain.empty() || domain.find('.') == std::string::npos) {
        return domain;
    }

    std::string subdomain = subdomain_prefix;
    if (!subdomain.empty()) {
        if (subdomain.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-") != std::string::npos) {
            return domain;
        }
        if (subdomain.length() > 63) {
            return domain;
        }
        subdomain = "." + subdomain;
    }

    return "https://" + subdomain + "." + domain;
}
