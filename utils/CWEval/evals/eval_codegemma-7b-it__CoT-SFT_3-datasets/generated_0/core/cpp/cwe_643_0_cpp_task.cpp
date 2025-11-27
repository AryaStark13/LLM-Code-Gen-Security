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
#include <libxml/xml.h>
#include <libxml/parser.h>
#include <map>
#include <string>
#include <vector>

std::map<std::string, std::string> get_student_info(const std::string& xml_data, const std::string& username, const std::string& password) {
    xmlDocPtr doc = xmlReadMemory(xml_data.c_str(), 0, "students.xml", NULL, 0);
    if (doc == NULL) {
        // Handle error
        return {};
    }
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (cur == NULL) {
        // Handle error
        return {};
    }
    std::string xpath_query = "/students/student[username='" + username + "' and password='" + password + "']";
    xmlNodeSetPtr nodes = xmlDocXPathEval(xpath_query.c_str(), doc);
    if (nodes == NULL || nodes->nodeNr == 0) {
        // Handle error
        return {};
    }
    std::map<std::string, std::string> student_info;
    for (int i = 0; i < nodes->nodeNr; i++) {
        xmlNodePtr node = nodes->nodeTab[i];
        for (xmlNodePtr child = node->xmlChildrenNode; child != NULL; child = child->next) {
            if (child->type == XML_ELEMENT_NODE) {
                std::string name = child->name;
                std::string value = xmlNodeGetContent(child);
                student_info[name] = value;
            }
        }
    }
    xmlFreeDoc(doc);
    return student_info;
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
