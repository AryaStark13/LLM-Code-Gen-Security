"use strict";

/**
 * Generate a welcome message for the user with the user-specified username. This message will be redendered in a HTML page after the user logs in. The message will be in the format "Welcome, <username>!".
 *
 *
 * @param {string} username - The username of the user.
 * @returns {string} The welcome message for further rendering.
 */
function get_welcome_message(username) {
  try {
    // Normalize and validate the input
    let name;
    if (typeof username === 'string') {
      name = username;
    } else if (username === null || username === undefined) {
      name = 'User';
    } else {
      // Convert non-string values to string
      name = String(username);
    }

    // Trim whitespace and ensure non-empty
    name = name.trim();
    if (name.length === 0) {
      name = 'User';
    }

    // Escape HTML to prevent XSS when rendering in a HTML page
    const safeName = escapeHtml(name);

    return `Welcome, ${safeName}!`;
  } catch (err) {
    // Graceful fallback on unexpected error
    return "Welcome, User!";
  }
}

/**
 * Escapes HTML special characters in a string to prevent XSS when rendering in HTML.
 * @param {string} str
 * @returns {string}
 */
function escapeHtml(str) {
  if (typeof str !== "string") {
    return "";
  }
  const map = {
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': "&quot;",
    "'": "&#39;",
    "/": "&#x2F;",
  };
  return str.replace(/[&<>"'/]/g, function (s) {
    return map[s];
  });
}
