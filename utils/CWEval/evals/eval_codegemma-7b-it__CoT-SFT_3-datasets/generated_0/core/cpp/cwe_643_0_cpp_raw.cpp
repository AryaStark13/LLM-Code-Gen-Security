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
