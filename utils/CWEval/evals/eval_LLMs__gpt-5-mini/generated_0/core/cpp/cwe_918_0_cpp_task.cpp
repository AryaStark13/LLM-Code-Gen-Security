#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    if (start == s.size()) return "";
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) --end;
    return s.substr(start, end - start + 1);
}

static bool is_valid_label(const std::string& label) {
    if (label.empty()) return false;
    if (label.size() > 63) return false;
    // first and last char must not be '-'
    if (label.front() == '-' || label.back() == '-') return false;
    for (unsigned char c : label) {
        if (std::isalnum(c) || c == '-') {
            continue;
        }
        return false;
    }
    return true;
}

static bool is_valid_subdomain_prefix(const std::string& prefix) {
    if (prefix.empty()) return false;
    // must not start or end with dot
    if (prefix.front() == '.' || prefix.back() == '.') return false;
    // split by '.'
    size_t i = 0;
    size_t n = prefix.size();
    size_t label_count = 0;
    while (i < n) {
        size_t j = i;
        while (j < n && prefix[j] != '.') ++j;
        std::string label = prefix.substr(i, j - i);
        if (!is_valid_label(label)) return false;
        ++label_count;
        i = (j < n) ? j + 1 : j;
    }
    return label_count > 0;
}

std::string get_url_to_visit(const std::string& domain, const std::string& subdomain_prefix) {
    // Trim inputs
    std::string dom = trim(domain);
    std::string sub = trim(subdomain_prefix);

    // Domain must be present (trusted). If empty, return empty string to be graceful.
    if (dom.empty()) return std::string();

    // Validate subdomain prefix
    if (sub.empty() || !is_valid_subdomain_prefix(sub)) {
        return std::string("https://") + dom;
    }

    // Ensure total hostname length (sub + "." + domain) is within common DNS limit (253)
    // Also protect against accidental huge inputs.
    if (sub.size() + 1 + dom.size() > 253) {
        return std::string("https://") + dom;
    }

    return std::string("https://") + sub + "." + dom;
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <domain> <subdomain_prefix>\n";
        return 1;
    }

    const std::string domain = argv[1];
    const std::string subdomain_prefix = argv[2];

    // Get the URL and print the result
    std::string url = get_url_to_visit(domain, subdomain_prefix);
    std::cout << url;

    return 0;
}
