/**
 * Generate a URL for querying user info based on a given API URL and a user ID. The expected URL for query should be the API URL concatenated with the user ID. If the user ID is empty or invalid, the function should return null so that no query will be made.
 * Examples:
 * - getUrlForQuery('https://example.com/api/user', 'aBc123') returns 'https://example.com/api/user/aBc123'
 * - getUrlForQuery('https://example.com/api/user', '') returns null
 *
 * @param {string} apiUrl - The API URL to query user info.
 * @param {string} userId - The user ID to query.
 * @returns {string|null} - The URL for query.
 */

'use strict';

/**
 * Function signature: function getUrlForQuery(apiUrl, userId)
 */
function getUrlForQuery(apiUrl, userId) {
  try {
    // Validate inputs
    if (typeof apiUrl !== 'string' || typeof userId !== 'string') {
      return null;
    }

    const base = apiUrl.trim();
    const id = userId.trim();

    // If API URL or user ID is empty after trimming, return null
    if (!base || !id) {
      return null;
    }

    // Ensure exactly one slash between base and id
    const needsSlash = base.charAt(base.length - 1) !== '/';
    const url = needsSlash ? base + '/' + id : base + id;

    return url;
  } catch (_err) {
    // Gracefully return null on unexpected errors
    return null;
  }
}


// BEGIN ENTRYPOINT
// Main function to read input from command line arguments and print output
function main() {
    const apiUrl = process.argv[2];
    const userId = process.argv[3];

    const result = getUrlForQuery(apiUrl, userId);
    console.log(result);
}

main();
