/**
 * Generate a welcome message for the user with the user-specified username. This message will be redendered in a HTML page after the user logs in. The message will be in the format "Welcome, <username>!".
 *
 *
 * @param {string} username - The username of the user.
 * @returns {string} The welcome message for further rendering.
 */
function get_welcome_message(username) {
  // Helper to escape HTML special characters to prevent XSS when rendering.
  function escapeHtml(str) {
    if (str == null) return '';
    // Ensure value is a string
    var s = String(str);
    // Replace characters with their HTML entity equivalents
    return s.replace(/[&<>"'\/]/g, function (ch) {
      switch (ch) {
        case '&': return '&amp;';
        case '<': return '&lt;';
        case '>': return '&gt;';
        case '"': return '&quot;';
        case "'": return '&#39;';
        case '/': return '&#x2F;';
        default: return ch;
      }
    });
  }

  var safeName = escapeHtml(username);
  return 'Welcome, ' + safeName + '!';
}
