#include <libxml/parser.h>
#include <libxml/xpath.h>

typedef struct {
    char* name;
    char* age;
    char* citizenship;
} StudentInfo;

StudentInfo* get_student_info(const char* xml_data, const char* username, const char* password) {
    xmlDocPtr doc = NULL;
    xmlNodePtr curNode = NULL;
    xmlXPathContextPtr context = NULL;
    xmlXPathObjectPtr result = NULL;
    StudentInfo* studentInfo = NULL;

    // Parse the XML data
    doc = xmlReadMemory(xml_data, -1, NULL, NULL, 0);
    if (doc == NULL) {
        // Handle error
        return NULL;
    }

    // Create a new XPath context
    context = xmlXPathNewContext(doc);
    if (context == NULL) {
        // Handle error
        xmlFreeDoc(doc);
        return NULL;
    }

    // Create a new XPath object
    result = xmlXPathEvalExpression(BAD_CAST "//student[username = 'username' and password = 'password']", context);
    if (result == NULL) {
        // Handle error
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return NULL;
    }

    // Check if a student is found
    if (result->nodesetval->nodeNr > 0) {
        // Get the first student node
        curNode = result->nodesetval->nodeTab[0];

        // Extract the student information
        studentInfo = (StudentInfo*)malloc(sizeof(StudentInfo));
        studentInfo->name = strdup((char*)xmlGetProp(curNode, (const xmlChar*)"name"));
        studentInfo->age = strdup((char*)xmlGetProp(curNode, (const xmlChar*)"age"));
        studentInfo->citizenship = strdup((char*)xmlGetProp(curNode, (const xmlChar*)"citizenship"));
    }

    // Free the XPath objects
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    return studentInfo;
}
