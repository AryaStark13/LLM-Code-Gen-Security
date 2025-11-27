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
        printf("Failed to parse XML\n");
        return NULL;
    }

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        printf("Failed to create XPath context\n");
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)"//student[username='"username"' and password='"password"']", context);
    if (result == NULL) {
        printf("Failed to evaluate XPath expression\n");
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlNodePtr node = result->nodesetval->nodeTab[0];
    if (node == NULL) {
        printf("No matching student found\n");
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return NULL;
    }

    StudentInfo* student = (StudentInfo*)malloc(sizeof(StudentInfo));
    student->name = (char*)xmlNodeGetContent(node->xmlChildrenNode);
    student->age = (char*)xmlNodeGetContent(node->next->xmlChildrenNode);
    student->citizenship = (char*)xmlNodeGetContent(node->next->next->xmlChildrenNode);

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    return student;
}
