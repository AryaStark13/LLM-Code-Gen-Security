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
