#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

typedef struct {
    char* name;
    char* age;
    char* citizenship;
} StudentInfo;

/* Helper: create an XPath string literal for arbitrary input.
 * The returned string is malloc'd and must be freed by the caller.
 * For example:
 *  - input: hello -> returns "'hello'"
 *  - input with single quotes but no double quotes -> "\"a'b\""
 *  - input with both kinds of quotes -> "concat('a',\"'\",\"b')"
 */
static char* xpath_escape_literal(const char* s) {
    if (s == NULL) {
        /* Represent empty with '' */
        char* r = (char*)malloc(3);
        if (r) { strcpy(r, "''"); }
        return r;
    }

    if (strchr(s, '\'') == NULL) {
        /* No single quotes, use single-quoted literal */
        size_t len = strlen(s);
        char* out = (char*)malloc(len + 3); /* ' ' and null */
        if (!out) return NULL;
        out[0] = '\'';
        memcpy(out + 1, s, len);
        out[len + 1] = '\'';
        out[len + 2] = '\0';
        return out;
    }

    if (strchr(s, '"') == NULL) {
        /* No double quotes, use double-quoted literal */
        size_t len = strlen(s);
        char* out = (char*)malloc(len + 3); /* " " and null */
        if (!out) return NULL;
        out[0] = '"';
        memcpy(out + 1, s, len);
        out[len + 1] = '"';
        out[len + 2] = '\0';
        return out;
    }

    /* Contains both single and double quotes.
     * Split on single quotes and build concat('seg1', "\"'\"", 'seg2', ...)
     */
    const char* p = s;
    size_t seg_count = 1;
    for (const char* q = s; *q; ++q) if (*q == '\'') ++seg_count;

    /* Collect segments */
    char** segs = (char**)malloc(sizeof(char*) * seg_count);
    if (!segs) return NULL;
    size_t idx = 0;
    const char* start = s;
    for (const char* q = s; ; ++q) {
        if (*q == '\'' || *q == '\0') {
            size_t seglen = (size_t)(q - start);
            segs[idx] = (char*)malloc(seglen + 1);
            if (!segs[idx]) {
                /* free previous */
                for (size_t j = 0; j < idx; ++j) free(segs[j]);
                free(segs);
                return NULL;
            }
            if (seglen) memcpy(segs[idx], start, seglen);
            segs[idx][seglen] = '\0';
            idx++;
            if (*q == '\0') break;
            start = q + 1;
        }
    }

    /* Compute required length for "concat(" + segments + separators + ")" + null */
    size_t total_len = strlen("concat(") + 1; /* for closing ')' later */
    for (size_t i = 0; i < seg_count; ++i) {
        total_len += 2; /* surrounding single quotes for segment */
        total_len += strlen(segs[i]);
        if (i + 1 < seg_count) {
            total_len += 4; /* ,"'", -> comma + double-quoted single quote (3 chars) */
        }
    }
    total_len += 1; /* null terminator */

    char* out = (char*)malloc(total_len);
    if (!out) {
        for (size_t j = 0; j < seg_count; ++j) free(segs[j]);
        free(segs);
        return NULL;
    }

    /* Build the concat(...) string */
    char* cur = out;
    const char* prefix = "concat(";
    strcpy(cur, prefix);
    cur += strlen(prefix);

    for (size_t i = 0; i < seg_count; ++i) {
        *cur++ = '\'';
        size_t sl = strlen(segs[i]);
        if (sl) {
            memcpy(cur, segs[i], sl);
            cur += sl;
        }
        *cur++ = '\'';
        if (i + 1 < seg_count) {
            /* append ,"'", where "'": double quote, single quote, double quote */
            *cur++ = ',';
            *cur++ = '"';
            *cur++ = '\'';
            *cur++ = '"';
        }
    }

    *cur++ = ')';
    *cur = '\0';

    for (size_t j = 0; j < seg_count; ++j) free(segs[j]);
    free(segs);
    return out;
}

/* Helper: evaluate a relative XPath expression in a context whose context node is already set.
 * Returns a newly allocated C string (malloc) with the node content, or NULL.
 * Caller must free the returned string.
 */
static char* xpath_get_relative_node_content(xmlXPathContextPtr ctx, const xmlChar* expr) {
    if (!ctx || !expr) return NULL;
    xmlXPathObjectPtr obj = xmlXPathEvalExpression(expr, ctx);
    if (!obj) return NULL;
    char* result = NULL;
    if (obj->type == XPATH_NODESET && obj->nodesetval && obj->nodesetval->nodeNr > 0) {
        xmlNodePtr node = obj->nodesetval->nodeTab[0];
        if (node) {
            xmlChar* content = xmlNodeGetContent(node);
            if (content) {
                /* Duplicate into malloc'd C string */
                size_t len = xmlStrlen(content);
                result = (char*)malloc(len + 1);
                if (result) {
                    memcpy(result, (char*)content, len);
                    result[len] = '\0';
                }
                xmlFree(content);
            }
        }
    }
    xmlXPathFreeObject(obj);
    return result;
}

StudentInfo* get_student_info(const char* xml_data, const char* username, const char* password) {
    if (!xml_data || !username || !password) return NULL;

    /* Escape username and password for XPath literal */
    char* user_lit = xpath_escape_literal(username);
    char* pass_lit = xpath_escape_literal(password);
    if (!user_lit || !pass_lit) {
        free(user_lit);
        free(pass_lit);
        return NULL;
    }

    /* Build XPath expression: //student[username/text()=user_lit and password/text()=pass_lit] */
    const char* xpath_fmt_start = "//student[username/text()=";
    const char* xpath_fmt_mid = " and password/text()=";
    const char* xpath_fmt_end = "]";
    size_t expr_len = strlen(xpath_fmt_start) + strlen(user_lit) + strlen(xpath_fmt_mid) + strlen(pass_lit) + strlen(xpath_fmt_end) + 1;
    char* xpath_expr = (char*)malloc(expr_len);
    if (!xpath_expr) {
        free(user_lit);
        free(pass_lit);
        return NULL;
    }
    strcpy(xpath_expr, xpath_fmt_start);
    strcat(xpath_expr, user_lit);
    strcat(xpath_expr, xpath_fmt_mid);
    strcat(xpath_expr, pass_lit);
    strcat(xpath_expr, xpath_fmt_end);

    /* Parse XML from memory */
    xmlDocPtr doc = xmlReadMemory(xml_data, (int)strlen(xml_data), "noname.xml", NULL, 0);
    if (!doc) {
        free(user_lit);
        free(pass_lit);
        free(xpath_expr);
        return NULL;
    }

    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    if (!xpathCtx) {
        xmlFreeDoc(doc);
        free(user_lit);
        free(pass_lit);
        free(xpath_expr);
        return NULL;
    }

    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar*)xpath_expr, xpathCtx);
    free(xpath_expr);
    free(user_lit);
    free(pass_lit);

    if (!xpathObj) {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return NULL;
    }

    StudentInfo* info = NULL;

    if (xpathObj->type == XPATH_NODESET && xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0) {
        /* Take the first matching student node */
        xmlNodePtr studentNode = xpathObj->nodesetval->nodeTab[0];
        if (studentNode) {
            /* Setup a new context that is the same but with node set to the student node */
            xmlXPathContextPtr relCtx = xmlXPathNewContext(doc);
            if (relCtx) {
                relCtx->node = studentNode;

                char* name = xpath_get_relative_node_content(relCtx, (xmlChar*)"name");
                char* citizenship = xpath_get_relative_node_content(relCtx, (xmlChar*)"citizenship");
                char* age = xpath_get_relative_node_content(relCtx, (xmlChar*)"age");

                if (name || citizenship || age) {
                    info = (StudentInfo*)malloc(sizeof(StudentInfo));
                    if (info) {
                        info->name = name;
                        info->citizenship = citizenship;
                        info->age = age;
                    } else {
                        /* allocation failed, free values */
                        if (name) free(name);
                        if (citizenship) free(citizenship);
                        if (age) free(age);
                    }
                } else {
                    /* No fields found */
                    /* leave info as NULL */
                }

                xmlXPathFreeContext(relCtx);
            }
        }
    }

    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    /* Note: not calling xmlCleanupParser() here to avoid global side-effects */

    return info;
}
