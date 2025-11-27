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
