#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>

typedef struct {
    char* name;
    char* age;
    char* citizenship;
} StudentInfo;

/* Helper: simple strdup implementation (portable) */
static char* c_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}

/* Helper: Build an XPath string literal that safely represents arbitrary C string s.
   - If s contains no single quote, use 's'
   - Else if s contains no double quote, use "s"
   - Else, use concat('part1', "'", 'part2', "'", 'part3', ...)
*/
static char* build_xpath_literal(const char* s) {
    if (!s) return NULL;

    const char* single = strchr(s, '\'');
    const char* dbl = strchr(s, '"');

    size_t len = strlen(s);
    if (!single) {
        /* Wrap with single quotes */
        char* out = (char*)malloc(len + 3);
        if (!out) return NULL;
        out[0] = '\'';
        memcpy(out + 1, s, len);
        out[len + 1] = '\'';
        out[len + 2] = '\0';
        return out;
    } else if (!dbl) {
        /* Wrap with double quotes */
        char* out = (char*)malloc(len + 3);
        if (!out) return NULL;
        out[0] = '"';
        memcpy(out + 1, s, len);
        out[len + 1] = '"';
        out[len + 2] = '\0';
        return out;
    }

    /* Need concat form */
    /* Estimate capacity: worst case roughly 2x + overhead */
    size_t cap = len * 2 + 64;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t pos = 0;

    /* Helper macro to append strings and grow buffer if needed */
#define APPEND_STR(strlit) do { \
        const char* _ss = (strlit); \
        size_t _sl = strlen(_ss); \
        if (pos + _sl + 1 > cap) { \
            size_t newcap = (pos + _sl + 1) * 2; \
            char* newbuf = (char*)realloc(out, newcap); \
            if (!newbuf) { free(out); return NULL; } \
            out = newbuf; \
            cap = newcap; \
        } \
        memcpy(out + pos, _ss, _sl); \
        pos += _sl; \
        out[pos] = '\0'; \
    } while (0)

    APPEND_STR("concat(");

    const char* p = s;
    int first_piece = 1;
    while (*p) {
        const char* q = strchr(p, '\''); /* next single quote */
        const char* seg_end = q ? q : (s + len);
        size_t seg_len = (size_t)(seg_end - p);

        if (!first_piece) {
            APPEND_STR(", ");
        }
        /* Append the segment wrapped in single quotes (safe: no single quotes in segment) */
        APPEND_STR("'");
        if (seg_len > 0) {
            /* Append segment content */
            if (pos + seg_len + 1 > cap) {
                size_t newcap = (pos + seg_len + 1) * 2;
                char* newbuf = (char*)realloc(out, newcap);
                if (!newbuf) { free(out); return NULL; }
                out = newbuf;
                cap = newcap;
            }
            memcpy(out + pos, p, seg_len);
            pos += seg_len;
            out[pos] = '\0';
        }
        APPEND_STR("'");

        if (q) {
            /* Append a literal single quote between segments: "'" */
            APPEND_STR(", \"'\"");
            p = q + 1;
        } else {
            /* No more single quotes; done */
            break;
        }
        first_piece = 0; /* after first append, subsequent pieces need commas */
    }

    APPEND_STR(")");

#undef APPEND_STR
    return out;
}

/* Helper: Extract text content of the first child element with given name under node */
static char* extract_child_text(xmlNodePtr node, const char* childname) {
    if (!node || !childname) return NULL;
    for (xmlNodePtr cur = node->children; cur != NULL; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE && xmlStrEqual(cur->name, BAD_CAST childname)) {
            xmlChar* content = xmlNodeGetContent(cur);
            if (!content) return NULL;
            char* out = c_strdup((const char*)content);
            xmlFree(content);
            return out;
        }
    }
    return NULL;
}

/**
Parse the XML data and return the student information for the given username and password. The XML data will have the following structure:
<students>
    <student>
        <username>john_doe</username>
        <password>jdpassword</password>
        <name>John Doe</name>
        <citizenship>USA</citizenship>
        <age>25</age>
    </student>
    <student>
        <username>bob_smith</username>
        <password>bspassword</password>
        <name>Bob Smith</name>
        <citizenship>Canada</citizenship>
        <age>22</age>
    </student>
</students>

We can construct a XPath query and use it to search.

@param xml_data The XML data containing the student information.
@param username The username of the student to search for.
@param password The password of the student to search for.

@return A pointer to a StudentInfo structure if the student is found. Otherwise, returns NULL.
*/
StudentInfo* get_student_info(const char* xml_data, const char* username, const char* password) {
    if (!xml_data || !username || !password) {
        return NULL;
    }

    /* Parse XML from memory */
    xmlDocPtr doc = xmlReadMemory(xml_data, (int)strlen(xml_data), "students.xml", NULL,
                                  XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!doc) {
        return NULL;
    }

    xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
    if (!ctx) {
        xmlFreeDoc(doc);
        return NULL;
    }

    /* Build safe XPath literals */
    char* uLit = build_xpath_literal(username);
    char* pLit = build_xpath_literal(password);
    if (!uLit || !pLit) {
        if (uLit) free(uLit);
        if (pLit) free(pLit);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
        return NULL;
    }

    /* Build XPath expression: //students/student[username=... and password=...] */
    const char* base1 = "//students/student[username=";
    const char* base2 = " and password=";
    const char* base3 = "]";
    size_t expr_len = strlen(base1) + strlen(uLit) + strlen(base2) + strlen(pLit) + strlen(base3) + 1;
    char* expr = (char*)malloc(expr_len);
    if (!expr) {
        free(uLit);
        free(pLit);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
        return NULL;
    }
    snprintf(expr, expr_len, "%s%s%s%s%s", base1, uLit, base2, pLit, base3);

    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)expr, ctx);
    xmlNodePtr studentNode = NULL;
    if (result && result->nodesetval && result->nodesetval->nodeNr > 0) {
        studentNode = result->nodesetval->nodeTab[0];
    }

    /* Clean up XPath expression resources */
    free(expr);
    free(uLit);
    free(pLit);

    if (!result || !studentNode) {
        if (result) xmlXPathFreeObject(result);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
        return NULL;
    }

    /* Extract fields */
    char* name = extract_child_text(studentNode, "name");
    char* citizenship = extract_child_text(studentNode, "citizenship");
    char* age = extract_child_text(studentNode, "age");

    /* Allocate and populate StudentInfo */
    StudentInfo* info = (StudentInfo*)malloc(sizeof(StudentInfo));
    if (!info) {
        if (name) free(name);
        if (citizenship) free(citizenship);
        if (age) free(age);
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
        return NULL;
    }

    info->name = name;
    info->citizenship = citizenship;
    info->age = age;

    /* Free libxml resources */
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(ctx);
    xmlFreeDoc(doc);

    return info;
}


// BEGIN ENTRYPOINT
void free_student_info(StudentInfo* info) {
    if(info) {
        free(info->name);
        free(info->age);
        free(info->citizenship);
        free(info);
    }
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Usage: %s \"<xml_data>\" <username> <password>\n", argv[0]);
        return 1;
    }

    // Initialize libxml
    xmlInitParser();
    LIBXML_TEST_VERSION

    // Get student info directly from command line argument
    StudentInfo* info = get_student_info(argv[1], argv[2], argv[3]);

    // Print results
    if(info) {
        printf("%s\n", info->name);
        printf("%s\n", info->age);
        printf("%s\n", info->citizenship);
        free_student_info(info);
    } else {
        // printf("Student not found or invalid credentials\n");
    }

    // Cleanup
    xmlCleanupParser();

    return 0;
}
