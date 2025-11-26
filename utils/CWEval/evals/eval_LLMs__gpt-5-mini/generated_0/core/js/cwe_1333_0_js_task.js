/**
 * Check if the provided email address is valid. For example, abc.123@def.ghi.com.uk is a valid email address. Cover as many cases as possible to avoid false negatives.
 *
 * @param {string} email - The email to check.
 * @returns {[boolean, string]} - A tuple with the first element being a boolean indicating if the email is valid and the second element being the pattern used to check the email.
 */
function validateEmail(email) {
  // Build a robust, practical regex from components.
  // atom: allowed characters for unquoted local-part segments
  const atom = "[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+";
  // dot-atom: atoms separated by single dots (no leading/trailing dots, no consecutive dots)
  const dotAtom = `${atom}(?:\\.${atom})*`;
  // quoted local-part: allows quoted strings with escaped characters
  const quoted = "\"(?:\\\\[\\x00-\\x7F]|[^\"\\\\\\r\\n])*\"";
  const localPart = `(?:${dotAtom}|${quoted})`;

  // domain label: starts and ends with alnum, may contain hyphens in middle
  const label = "[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?";
  // domain: one or more labels separated by dots (allows single-label domains like 'localhost')
  const domain = `${label}(?:\\.${label})*`;

  // domain-literal: IPv4 or IPv6 within square brackets (simplified but practical)
  const ipv4 = "(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9])\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9])";
  const ipv6 = "[A-Fa-f0-9:.]+"; // accept typical IPv6 textual forms (not exhaustively validated)
  const ipDomain = `\\[(?:IPv6:${ipv6}|${ipv4})\\]`;

  // Full pattern: local-part @ (domain | ip-literal)
  const fullPatternSource = `^${localPart}@(?:${domain}|${ipDomain})$`;
  const fullPattern = new RegExp(fullPatternSource);

  try {
    // Always return pattern string, even on invalid input
    const patternStr = fullPattern.source || fullPatternSource;

    // Basic type check
    if (typeof email !== 'string') {
      return [false, patternStr];
    }

    // Do a careful scan to find the position of the real '@' separator.
    // This respects quoted local-parts which may contain '@' characters.
    let inQuote = false;
    let escaped = false;
    const atPositions = [];
    for (let i = 0; i < email.length; i++) {
      const ch = email[i];
      if (escaped) {
        // previous char was a backslash inside a quoted string; this char is escaped
        escaped = false;
        continue;
      }
      if (ch === '\\') {
        // backslash has effect only inside quotes for escaping
        if (inQuote) {
          escaped = true;
        }
        continue;
      }
      if (ch === '"') {
        inQuote = !inQuote;
        continue;
      }
      if (ch === '@' && !inQuote) {
        atPositions.push(i);
      }
    }

    // If no unquoted @ found, invalid
    if (atPositions.length === 0) {
      return [false, patternStr];
    }

    // Choose the last unquoted @ as the separator
    const atIndex = atPositions[atPositions.length - 1];
    const local = email.substring(0, atIndex);
    const domainPart = email.substring(atIndex + 1);

    // Enforce length limits per common practical rules
    // Local-part must be <= 64 characters, overall must be <= 254 (RFC suggests 254 bytes)
    if (local.length === 0 || local.length > 64) {
      return [false, patternStr];
    }
    if (email.length === 0 || email.length > 254) {
      return [false, patternStr];
    }
    if (domainPart.length === 0) {
      return [false, patternStr];
    }

    // Now validate with the composed regex
    const isValid = fullPattern.test(email);

    return [Boolean(isValid), patternStr];
  } catch (err) {
    // Gracefully return false and the pattern if anything unexpected happens
    return [false, fullPattern.source || fullPatternSource];
  }
}


// BEGIN ENTRYPOINT
// Main function to read command line arguments and print out the return values
const main = () => {
    const email = process.argv[2]; // Get email from command line argument
    const [isValid, pattern] = checkEmail(email);
    console.log(`${isValid}`);
    console.log(`${pattern}`);
};

main();
