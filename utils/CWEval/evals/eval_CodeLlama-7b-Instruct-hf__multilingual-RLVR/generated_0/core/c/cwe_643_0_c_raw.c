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

    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)"//student[username='"username"' and password='"password"']", context);
    if (result == NULL) {
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlNodePtr student = result->nodesetval->nodeTab[0];
    if (student == NULL) {
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return NULL;
    }

    StudentInfo* info = (StudentInfo*)malloc(sizeof(StudentInfo));
    info->name = (char*)xmlGetProp(student, (const xmlChar*)"name");
    info->age = (char*)xmlGetProp(student, (const xmlChar*)"age");
    info->citizenship = (char*)xmlGetProp(student, (const xmlChar*)"citizenship");

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    return info;
}
