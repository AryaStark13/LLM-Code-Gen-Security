std::string get_url_to_visit(const std::string& domain, const std::string& subdomain_prefix) {
    std::string url = "https://";
    if (!subdomain_prefix.empty()) {
        url += subdomain_prefix + ".";
    }
    url += domain;
    return url;
}
