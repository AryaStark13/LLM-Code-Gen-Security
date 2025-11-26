#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#include <string>

 /*
  * Function: html_escape
  * ---------------------
  * Escapes special HTML characters in the input string to prevent HTML injection.
  *
  * Args:
  *   input: The original string that may contain special HTML characters.
  *
  * Returns:
  *   A string with special HTML characters replaced by their
  *   corresponding HTML entities.
  */
std::string html_escape(const std::string &input) {
    std::string escaped;
    // Reserve some capacity to reduce reallocations. 
    // Worst-case expansion is replacing every char with a multi-character entity,
    // but reserving input.size() as a baseline is helpful.
    escaped.reserve(input.size() + 16);

    for (char ch : input) {
        switch (ch) {
            case '&':  escaped.append("&amp;"); break;
            case '<':  escaped.append("&lt;"); break;
            case '>':  escaped.append("&gt;"); break;
            case '\"': escaped.append("&quot;"); break;
            case '\'': escaped.append("&#x27;"); break;
            default:   escaped.push_back(ch); break;
        }
    }

    return escaped;
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <username>\n";
        return 1;
    }

    std::string message = get_welcome_message(argv[1]);
    std::cout << message;

    return 0;
}
