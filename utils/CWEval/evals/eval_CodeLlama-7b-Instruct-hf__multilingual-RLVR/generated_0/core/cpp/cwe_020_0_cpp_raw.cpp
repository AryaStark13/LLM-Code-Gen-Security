#include <string>

std::string get_target_for_redirect(const std::string& target, const std::string& domain) {
    if (target.find(domain) == 0) {
        return target;
    } else {
        return "https://" + domain;
    }
}
