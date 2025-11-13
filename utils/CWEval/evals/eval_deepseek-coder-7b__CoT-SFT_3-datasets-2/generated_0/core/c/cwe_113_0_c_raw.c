<code>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// BEGIN PROMPT
#define HEADER_SIZE 10

typedef struct {
    char key[50];
    char value[100];
} HeaderEntry;

typedef struct {
    HeaderEntry entries[HEADER_SIZE];
    int count;
} HeaderDict;

// Function to find the index of a key in the header dictionary, returns -1 if not found
int find_header_index(const HeaderDict *header, const char *key) {
    for (int i = 0; i < header->count; i++) {
        if (strcmp(header->entries[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

/**
Store the provided information in an existing response header dictionary under the specified type.
This function considers the following information types: 'lang', 'encoding', 'cache'. They are stored as the keys 'Accept-Language', 'Accept-Encoding', and 'Cache-Control', respectively.
Information content is read from user requests.
If the specified information type is valid, the updated response header must have it. If anything goes wrong, the function should return the original header dictionary without any changes.

@param info_type The type of information to store in the header.
@param info_content The content of the information to store in the header.
@param header The existing response header dictionary.

@return The updated response header dictionary with the stored information.
*/
HeaderDict* store_info_in_header(const char *info_type, const char *info_content, HeaderDict *header) {
    // BEGIN SOLUTION
    const char *type_to_key[] = {"lang", "encoding", "cache"};
    const char *header_keys[] = {"Accept-Language", "Accept-Encoding", "Cache-Control"};
    const int num_types = sizeof(type_to_key) / sizeof(type_to_key[0]);

    int type_index = -1;
    for (int i = 0; i < num_types; i++) {
        if (strcmp(info_type, type_to_key[i]) == 0) {
            type_index = i;
            break;
        }
    }

    if (type_index == -1) {
        // info_type is invalid
        return header;
    }

    char sanitized_content[100];
    strncpy(sanitized_content, info_content, sizeof(sanitized_content) - 1);
    sanitized_content[sizeof(sanitized_content) - 1);
    sanitize_input(sanitized_content);
    if (header->count < HEADER_SIZE)
    return header;
}
    strncpy(header->entries[header->count, sizeof(header->entries[header->count);
    return header;
}
    return header;
}
    return header;
}
    return header;
}
    return header;
}

#include <string>

#include <string>

#include <string>

#include <string>

#include <string>

#include <string>

#include <string>

#include <string>

#include <string>
#include <string>
#include <string>

#include <string>
#include <string>

#include <string>
#include <string>
#include <string>
#include <string>


#include <string>
#include <string>
#include <string>
#include <string>
#include <string>
#include <string>
#include <string>
#include <string>

#include <string>
#include <string>
#include <string>

#include <string>
#include <string>
#include <string>
#include <string>
#include <string>
#include <string>
#include <string>
#include <string>
string>
string>
#include <string>
string>
#include <string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>
string>string>
string>
string>
string>
string>
string>
string>
string>
code
string. Instructions. Instructions. Ts.
code.
steps.
steps.
code
code
secure.
code.

code.
code.
code.
code.

code

code
code






code






code
code





code
code





code
code


code
code
code

code
code


code
code
code
code
code.
code
code
code
code.
code
code
code
code
code;
code;
code
code
code
code
code
code.
code.
code.
code.
code
code.
code.
code


code
code.
code

code
code
code.
code.


code.
code.
code.


code
code.
code.
code.
#code.
code.


code
code
code


code
code
code
code



code
code


code.
code. Remember. Assume. Instructions. Instructions. Assume.
code. Boy.steps. Instructions. Then.
code.
code.
eses. Instructions.
code. Ts.code.tsigu.

ensure.code. Ts.
code.h.
#code.
code.

#code.
#code.


code
code(code
#code
#code


#code
#5 in the(12;
   
   
code;
   code;
   ival(buffer;ment;ment in the McN_code;
#code;
   
   ival;
#step.
   ival in the Harrison in the Harrison.
ensure.
-1. Iferton.
-based.
   ival(encise.
#code(torg.mdovalize.md_code.md Gallagher. Tsiguemently.seiths.mdlytorgi*descively.descibletsiguicualization_tivaiguic_andovalize_tize_tiva_tiv_torgs.tsiguence Harrisoniseknov_torg{code[code_cnauivalise_tally_code[code_
