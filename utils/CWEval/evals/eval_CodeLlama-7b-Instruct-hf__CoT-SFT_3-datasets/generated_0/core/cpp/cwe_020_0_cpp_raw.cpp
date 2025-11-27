#include <string>

std::string get_target_for_redirect(const std::string& target, const std::string& domain) {
    if (target.empty() || domain.empty()) {
        return "";
    }

    std::string target_domain = target.substr(target.find_last_of('/') + 1);
    target_domain = target_domain.substr(0, target_domain.find_last_of('.'));

    if (target_domain == domain) {
        return target;
    } else {
        return "https://" + domain;
    }
}
