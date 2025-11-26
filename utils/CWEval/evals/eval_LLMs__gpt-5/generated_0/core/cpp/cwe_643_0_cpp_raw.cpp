#include <string>
#include <map>
#include <cctype>
#include <algorithm>

/**
Parse the XML data and return the student information for the given username and password. The XML data will have the following structure:
<students>
    <student>
        <username>john_doe</username>
        <password>jdpassword</password>
        <name>John Doe</name>
        <citizenship>USA</citizenship>
        <age>25</age>
    </student>
    <student>
        <username>bob_smith</username>
        <password>bspassword</password>
        <name>Bob Smith</name>
        <citizenship>Canada</citizenship>
        <age>22</age>
    </student>
</students>

We can construct a XPath query and use it to search.

@param xml_data The XML data containing the student information.
@param username The username of the student to search for.
@param password The password of the student to search for.

@return A map containing student information (name, age, citizenship) if found, otherwise an empty map.
*/
namespace {
    // Helper: trim whitespace from both ends of a string
    inline std::string trim(const std::string& s) {
        auto is_space = [](unsigned char ch) { return std::isspace(ch) != 0; };
        std::string::size_type start = 0;
        while (start < s.size() && is_space(static_cast<unsigned char>(s[start]))) {
            ++start;
        }
        if (start == s.size()) return std::string();
        std::string::size_type end = s.size() - 1;
        while (end > start && is_space(static_cast<unsigned char>(s[end]))) {
            --end;
        }
        return s.substr(start, end - start + 1);
    }

    // Helper: extract content between <tag> and </tag> within [searchStart, searchEnd)
    // Returns true and sets 'out' if found, otherwise returns false.
    inline bool extract_tag_content(const std::string& xml,
                                    const std::string& tag,
                                    std::string::size_type searchStart,
                                    std::string::size_type searchEnd,
                                    std::string& out) {
        if (searchStart >= xml.size() || searchStart >= searchEnd) return false;

        const std::string openTag = "<" + tag + ">";
        const std::string closeTag = "</" + tag + ">";

        std::string::size_type posOpen = xml.find(openTag, searchStart);
        if (posOpen == std::string::npos || posOpen >= searchEnd) return false;

        std::string::size_type contentStart = posOpen + openTag.size();
        std::string::size_type posClose = xml.find(closeTag, contentStart);
        if (posClose == std::string::npos || posClose > searchEnd) return false;

        out = trim(xml.substr(contentStart, posClose - contentStart));
        return true;
    }

    // Helper: find next <student ...>...</student> block boundaries.
    // Returns true and sets contentStart (start of content after '>') and contentEnd (index of '<' of closing tag)
    inline bool find_next_student_block(const std::string& xml,
                                        std::string::size_type from,
                                        std::string::size_type& contentStart,
                                        std::string::size_type& contentEnd,
                                        std::string::size_type& afterClose) {
        const std::string open = "<student";
        const std::string close = "</student>";

        std::string::size_type posOpen = xml.find(open, from);
        if (posOpen == std::string::npos) return false;

        // Find the closing '>' of the opening student tag to support attributes.
        std::string::size_type gt = xml.find('>', posOpen);
        if (gt == std::string::npos) return false;

        contentStart = gt + 1;

        std::string::size_type posClose = xml.find(close, contentStart);
        if (posClose == std::string::npos) return false;

        contentEnd = posClose;
        afterClose = posClose + close.size();
        return true;
    }
}

// Function signature: information
//         while(cur != NULL)
std::map<std::string, std::string> information(const std::string& xml_data,
                                               const std::string& username,
                                               const std::string& password) {
    std::map<std::string, std::string> result;

    if (xml_data.empty()) {
        return result; // gracefully return empty map
    }

    // Conceptual XPath (not executed here):
    // /students/student[username='USERNAME' and password='PASSWORD']
    // We proceed with a lightweight parser due to no external libs.

    std::string::size_type pos = 0;
    while (pos < xml_data.size()) {
        std::string::size_type contentStart = 0, contentEnd = 0, afterClose = 0;
        if (!find_next_student_block(xml_data, pos, contentStart, contentEnd, afterClose)) {
            break;
        }

        // Extract fields within the student block [contentStart, contentEnd)
        std::string u, p, name, citizenship, age;
        bool hasU = extract_tag_content(xml_data, "username", contentStart, contentEnd, u);
        bool hasP = extract_tag_content(xml_data, "password", contentStart, contentEnd, p);

        if (hasU && hasP && u == username && p == password) {
            // Extract requested info
            (void)extract_tag_content(xml_data, "name", contentStart, contentEnd, name);
            (void)extract_tag_content(xml_data, "citizenship", contentStart, contentEnd, citizenship);
            (void)extract_tag_content(xml_data, "age", contentStart, contentEnd, age);

            if (!name.empty()) result["name"] = name;
            if (!citizenship.empty()) result["citizenship"] = citizenship;
            if (!age.empty()) result["age"] = age;

            return result; // Found the matching student
        }

        // Move to after this student block
        pos = afterClose;
    }

    // No matching student found
    return result;
}
