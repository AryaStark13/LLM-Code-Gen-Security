#include <map>
#include <string>

void store_info_in_header(const std::string &info_type, const std::string &info_content, std::map<std::string, std::string> &header) {
    // Define the valid information types
    std::map<std::string, std::string> valid_info_types = {
        {"lang", "Accept-Language"},
        {"encoding", "Accept-Encoding"},
        {"cache", "Cache-Control"}
    };

    // Check if the information type is valid
    if (valid_info_types.find(info_type) != valid_info_types.end()) {
        // Store the information in the header map
        header[valid_info_types[info_type]] = info_content;
    }
}
