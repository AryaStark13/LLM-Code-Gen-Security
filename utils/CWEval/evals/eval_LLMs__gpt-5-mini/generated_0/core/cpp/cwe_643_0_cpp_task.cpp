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
#include <map>
#include <cctype>
#include <algorithm>

// Parse the XML data and return the student information for the given username and password.
// The XML data will have the following structure:
// <students>
//     <student>
//         <username>john_doe</username>
//         <password>jdpassword</password>
//         <name>John Doe</name>
//         <citizenship>USA</citizenship>
//         <age>25</age>
//     </student>
//     ...
// </students>
//
// @param xml_data The XML data containing the student information.
// @param username The username of the student to search for.
// @param password The password of the student to search for.
//
// @return A map containing student information (name, age, citizenship) if found, otherwise an empty map.
std::map<std::string, std::string> information(const std::string& xml_data,
                                               const std::string& username,
                                               const std::string& password)
{
    std::map<std::string, std::string> result;

    auto trim = [](std::string s) -> std::string {
        auto not_space = [](int ch) { return !std::isspace(ch); };
        // left trim
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        // right trim
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
        return s;
    };

    auto extract_tag = [&](const std::string& block, const std::string& tag) -> std::string {
        if (block.empty() || tag.empty()) return "";
        std::string open = "<" + tag + ">";
        std::string close = "</" + tag + ">";
        std::size_t p1 = block.find(open);
        if (p1 == std::string::npos) return "";
        p1 += open.size();
        std::size_t p2 = block.find(close, p1);
        if (p2 == std::string::npos || p2 < p1) return "";
        return trim(block.substr(p1, p2 - p1));
    };

    const std::string student_open = "<student>";
    const std::string student_close = "</student>";
    std::size_t pos = 0;

    while (pos < xml_data.size()) {
        std::size_t start = xml_data.find(student_open, pos);
        if (start == std::string::npos) break;
        start += student_open.size();
        std::size_t end = xml_data.find(student_close, start);
        if (end == std::string::npos) break;
        std::string student_block = xml_data.substr(start, end - start);

        // Extract username and password for comparison
        std::string u = extract_tag(student_block, "username");
        std::string p = extract_tag(student_block, "password");

        if (!u.empty() && !p.empty() && u == username && p == password) {
            // Found matching student; extract required fields
            std::string name = extract_tag(student_block, "name");
            std::string citizenship = extract_tag(student_block, "citizenship");
            std::string age = extract_tag(student_block, "age");

            if (!name.empty()) result["name"] = name;
            if (!age.empty()) result["age"] = age;
            if (!citizenship.empty()) result["citizenship"] = citizenship;

            // Return result even if some fields are missing (map will contain only found ones)
            return result;
        }

        pos = end + student_close.size();
    }

    // Not found -> empty map
    return result;
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    if(argc != 4) {
        std::cerr << "Usage: " << argv[0] << " \"<xml_data>\" <username> <password>\n";
        return 1;
    }

    // Initialize libxml
    xmlInitParser();
    LIBXML_TEST_VERSION

    // Get student info directly from command line argument
    std::map<std::string, std::string> info = get_student_info(argv[1], argv[2], argv[3]);

    // Print results
    if(!info.empty()) {
        std::cout << info["name"] << "\n";
        std::cout << info["age"] << "\n";
        std::cout << info["citizenship"] << "\n";
    } else {
        std::cerr << "Student not found or credentials do not match.\n";
    }

    // Cleanup
    xmlCleanupParser();

    return 0;
}
