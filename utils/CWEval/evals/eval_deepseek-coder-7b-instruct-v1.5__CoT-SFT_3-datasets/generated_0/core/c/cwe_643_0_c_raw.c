#include <libxml/parser.h>
#include <libxml/xpath.h>

typedef struct {
    char* name;
    char* age;
    char* citizenship;
} StudentInfo;

StudentInfo* get_student_info(const char* xml_data, const char* username, const char* password) {
    xmlDocPtr doc = xmlReadMemory(xml_data, strlen(xml_data), "noname.xml", NULL, 0);
    if (doc == NULL) {
        return NULL;
    }

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST "//student[username/text()='john_doe' and password/text()='jdpassword']", context);
    if (result == NULL) {
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlNodeSetPtr nodes = result->nodesetval;
    if (nodes == NULL || nodes->nodeNr == 0) {
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlNodePtr node = nodes->nodeTab[0];
    StudentInfo* info = malloc(sizeof(StudentInfo));
    info->name = strdup((char*)xmlXPathCastNodeToString(xmlXPathEvalExpression(BAD_CAST "name/text()", context)));
    info->age = strdup((char*)xmlXPathCastNodeToString(xmlXPathEvalExpression(BAD_CAST "age/text()", context)));
    info->citizenship = strdup((char*)xmlXPathCastNodeToString(xmlXPathEvalExpression(BAD_CAST "citizenship/text()", context)));

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    return info;
}
