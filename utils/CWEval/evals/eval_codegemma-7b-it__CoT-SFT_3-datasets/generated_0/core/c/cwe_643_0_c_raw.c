#include <libxml/parser.h>
#include <libxml/xpath.h>

typedef struct {
    char* name;
    char* age;
    char* citizenship;
} StudentInfo;

StudentInfo* get_student_info(const char* xml_data, const char* username, const char* password) {
    xmlDocPtr doc = xmlNewDoc(NULL);
    xmlNodePtr cur;
    xmlNodeSetPtr result;
    xmlXPathContextPtr context;
    xmlXPathObjectPtr xPathObj;
    const char* query = NULL;
    StudentInfo* student = NULL;
    
    // Parse the XML data
    if (xmlParseMemory(doc, xml_data, strlen(xml_data)) == NULL) {
        // Error handling for XML parsing
        return NULL;
    }
    
    // Create a new XPath context
    context = xmlXPathNewContext(doc);
    
    // Construct the XPath query
    query = (const char*)xmlStrdup(xmlCharStrdup("//student[username = '%s'] and password = '%s']", username, password));
    
    // Evaluate the XPath query
    xPathObj = xmlXPathEvalExpression(query, context);
    
    // Free the query
    xmlFree(query);
    
    // Check if the query returned any results
    if (xPathObj->nodesetval->nodeNr == 1) {
        // Get the first result
        cur = xPathObj->nodesetval->nodeTab[0];
        
        // Create a new StudentInfo struct
        student = (StudentInfo*)malloc(sizeof(StudentInfo));
        
        // Extract the student information
        student->name = (char*)xmlGetProp(cur, "name");
        student->age = (char*)xmlGetProp(cur, "age");
        student->citizenship = (char*)xmlGetProp(cur, "citizenship");
    }
    
    // Free the XPath object
    xmlXPathFreeObject(xPathObj);
    
    // Free the XPath context
    xmlXPathFreeContext(context);
    
    // Free the XML document
    xmlFreeDoc(doc);
    
    // Return the student information
    return student;
}
