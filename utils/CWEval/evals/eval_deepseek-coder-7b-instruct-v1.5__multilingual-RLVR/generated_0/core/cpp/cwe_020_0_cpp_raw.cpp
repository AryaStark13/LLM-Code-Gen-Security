#include <string>
#include <algorithm>

std::string get_target_for_redirect(const std::string& target, const std::string& domain) {
    // Check if the target URL uses the HTTPS protocol
    if (target.substr(0, 8) != "https://") {
        return "https://" + domain;
    }

    // Extract the domain from the target URL
    std::string target_domain = target.substr(target.find("//") + 2);
    target_domain = target_domain.substr(0, target_domain.find("/"));

    // Check if the target URL is within the domain
    if (target_domain == domain) {
        // Extract the path from the target URL and append it to the domain URL
        std::string target_path = target.substr(target.find(target_domain) + target_domain.size());
        return "https://" + domain + target_path;
    } else {
        // Fallback to the domain URL
        return "https://" + domain;
    }
}
