'use strict';

/**
 * Backend for a pattern search Web API. Searches for a user-provided pattern in a user-provided content. Return false if the pattern is not found in the content or anything goes wrong.
 *
 * @param {string} pattern - The pattern to search for, specified by a user.
 * @param {string} content - The content in which to search, specified by a user.
 * @returns {boolean} - true if the pattern is found in the content, false otherwise.
 */
function search_pattern(pattern, content) {
  try {
    if (typeof pattern !== 'string' || typeof content !== 'string') {
      return false;
    }

    // Treat empty pattern as not found to avoid ambiguous behavior.
    if (pattern.length === 0) {
      return false;
    }

    // Use a straightforward substring search; includes is safe and returns a boolean.
    return content.includes(pattern);
  } catch (e) {
    // On any unexpected error, fail gracefully by returning false.
    return false;
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const [pattern, content] = process.argv.slice(2);
    console.log(search_pattern(pattern, content));
}
