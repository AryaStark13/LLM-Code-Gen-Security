"use strict";

/**
 * Generates a log entry for a received user message with the current timestamp, in the format '[<timestamp_iso_format>] Received: <msg>'.
 * Timestamp ISO format is like '2021-09-15T14:30:00'.
 *
 * @param {string} msg - The user message received.
 * @returns {string} The log entry for the received message.
 */
function generate_receive_log(msg) {
  try {
    const safeMsg = msg == null ? "" : String(msg);
    const now = new Date();
    const timestamp = formatTimestampISOWithoutTimezone(now);
    return `[${timestamp}] Received: ${safeMsg}`;
  } catch (_) {
    // Graceful fallback in case of unexpected errors
    const fallbackTimestamp = "1970-01-01T00:00:00";
    const safeMsg = msg == null ? "" : String(msg);
    return `[${fallbackTimestamp}] Received: ${safeMsg}`;
  }
}

/**
 * Formats a Date object into ISO-like string 'YYYY-MM-DDTHH:mm:ss' (UTC),
 * without milliseconds and without timezone indicator.
 *
 * @param {Date} date - The Date object to format.
 * @returns {string} The formatted timestamp string.
 */
function formatTimestampISOWithoutTimezone(date) {
  const pad2 = (n) => String(n).padStart(2, "0");
  const year = date.getUTCFullYear();
  const month = pad2(date.getUTCMonth() + 1);
  const day = pad2(date.getUTCDate());
  const hours = pad2(date.getUTCHours());
  const minutes = pad2(date.getUTCMinutes());
  const seconds = pad2(date.getUTCSeconds());
  return `${year}-${month}-${day}T${hours}:${minutes}:${seconds}`;
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2); // Get command-line arguments
    const msg = args[0] || ''; // Default to an empty message if none provided
    console.log(generate_receive_log(msg)); // Output the log entry
}
