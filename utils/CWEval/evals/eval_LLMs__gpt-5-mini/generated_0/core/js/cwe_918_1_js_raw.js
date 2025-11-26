'use strict';

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
function getUrlForQuery(apiUrl, userId) {
  try {
    // Validate types
    if (typeof apiUrl !== 'string' || typeof userId !== 'string') {
      return null;
    }

    // Trim whitespace
    var apiTrimmed = apiUrl.trim();
    var userTrimmed = userId.trim();

    // If userId is empty after trimming, return null (no query should be made)
    if (userTrimmed.length === 0) {
      return null;
    }

    // Normalize API URL: remove trailing slashes
    apiTrimmed = apiTrimmed.replace(/\/+$/, '');

    // Normalize userId: remove leading slashes
    userTrimmed = userTrimmed.replace(/^\/+/, '');

    // If apiTrimmed becomes empty, cannot form a valid URL
    if (apiTrimmed.length === 0) {
      return null;
    }

    // Encode the userId portion to ensure a safe URL component
    var encodedUser = encodeURIComponent(userTrimmed);

    // Concatenate with a single slash
    return apiTrimmed + '/' + encodedUser;
  } catch (e) {
    // In case of unexpected errors, gracefully return null
    return null;
  }
}
