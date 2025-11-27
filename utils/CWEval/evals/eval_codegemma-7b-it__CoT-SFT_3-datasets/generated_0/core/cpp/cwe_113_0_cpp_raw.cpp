#include <map>
#include <string>

void store_info_in_header(const std::string &info_type, const std::string &info_content, std::map<std::string, std::string> &header) {
    // Check if the specified information type is valid
    if (info_type == "lang") {
        // Store the information in the header
        if (header.find("Accept-Language") == header.end()) {
            header["Accept-Language"] = info_content;
        } else {
            header["Accept-Language"] += ", " + info_content;
        }
    } else if (info_type == "encoding") {
        // Store the information in the header
        if (header.find("Accept-Encoding") == header.end()) {
            header["Accept-Encoding"] = info_content;
        } else {
            header["Accept-Encoding"] += ", " + info_content;
        }
    } else if (info_type == "cache") {
        // Store the information in the header
        if (header.find("Cache-Control") == header.end()) {
            header["Cache-Control"] = info_content;
        } else {
            header["Cache-Control"] += ", " + info_content;
        }
    }
}
