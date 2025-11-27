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
#include <iostream>
#include <string>
#include <map>
#include <libxml/parser.h>
#include <libxml/xpath.h>

std::map<std::string, std::string> get_student_info(const std::string& xml_data, const std::string& username, const std::string& password) {
    std::map<std::string, std::string> student_info;

    xmlDocPtr doc = xmlReadMemory(xml_data.c_str(), xml_data.size(), "noname.xml", NULL, 0);
    if (doc == NULL) {
        std::cerr << "Failed to parse XML data." << std::endl;
        return student_info;
    }

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        std::cerr << "Failed to create XPath context." << std::endl;
        xmlFreeDoc(doc);
        return student_info;
    }

    xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST "//student[username/text()='" + username + "' and password/text()='" + password + "']", context);
    if (result == NULL) {
        std::cerr << "Failed to evaluate XPath expression." << std::endl;
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return student_info;
    }

    int nodes_count = xmlXPathNodeSetGetLength(result->nodesetval);
    if (nodes_count > 0) {
        xmlNodeSetPtr nodes = result->nodesetval;
        for (int i = 0; i < nodes_count; i++) {
            xmlNodePtr node = nodes[i];
            xmlNodePtr name_node = xmlFirstElementChild(node);
            xmlNodePtr age_node = xmlNextElementSibling(name_node);
            xmlNodePtr citizenship_node = xmlNextElementSibling(age_node);

            student_info["name"] = reinterpret_cast<const char*>(name_node->xmlChildrenNode->content);
            student_info["age"] = reinterpret_cast<const char*>(age_node->xmlChildrenNode->content);
            student_info["citizenship"] = reinterpret_cast<const char*>(citizenship_node->xmlChildrenNode->content);
        }
    }

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
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
