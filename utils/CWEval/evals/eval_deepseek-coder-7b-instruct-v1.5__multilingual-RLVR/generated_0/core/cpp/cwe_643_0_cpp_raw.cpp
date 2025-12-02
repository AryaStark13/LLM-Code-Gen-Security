#include <map>
#include <string>
#include <libxml/parser.h>
#include <libxml/xpath.h>

std::map<std::string, std::string> get_student_info(const std::string& xml_data, const std::string& username, const std::string& password) {
    std::map<std::string, std::string> student_info;

    // Parse the XML data
    xmlDocPtr doc = xmlReadMemory(xml_data.c_str(), xml_data.size(), "noname.xml", NULL, 0);
    if (doc == NULL) {
        return student_info;
    }

    // Create the XPath context
    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        xmlFreeDoc(doc);
        return student_info;
    }

    // Construct the XPath query
    std::string xpath_query = "//student[username='" + username + "' and password='" + password + "']";
    xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST xpath_query.c_str(), context);
    if (result == NULL) {
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return student_info;
    }

    // Iterate over the result nodes
    xmlNodeSetPtr nodes = result->nodesetval;
    for (int i = 0; i < nodes->nodeNr; i++) {
        xmlNodePtr node = nodes->nodeTab[i];
        xmlChar* name = xmlGetProp(node, BAD_CAST "name");
        xmlChar* age = xmlGetProp(node, BAD_CAST "age");
        xmlChar* citizenship = xmlGetProp(node, BAD_CAST "citizenship");

        if (name && age && citizenship) {
            student_info["name"] = reinterpret_cast<const char*>(name);
            student_info["age"] = reinterpret_cast<const char*>(age);
            student_info["citizenship"] = reinterpret_cast<const char*>(citizenship);
        }

        if (name) xmlFree(name);
        if (age) xmlFree(age);
        if (citizenship) xmlFree(citizenship);
    }

    // Free the result and the context
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    return student_info;
}
