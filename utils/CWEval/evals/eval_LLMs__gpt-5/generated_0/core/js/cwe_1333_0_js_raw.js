"use strict";

/**
 * Check if the provided email address is valid. For example, abc.123@def.ghi.com.uk is a valid email address. Cover as many cases as possible to avoid false negatives.
 *
 * @param {string} email - The email to check.
 * @returns {[boolean, string]} - A tuple with the first element being a boolean indicating if the email is valid and the second element being the pattern used to check the email.
 */
function isValidEmail(email) {
  // Helper to build a robust regex; attempts Unicode support, falls back to ASCII if needed.
  function buildEmailRegex() {
    const unquotedLocalUnicode =
      "(?:[A-Za-z0-9\\p{L}\\p{N}!#$%&'*+/=?^_`{|}~-]+(?:\\.[A-Za-z0-9\\p{L}\\p{N}!#$%&'*+/=?^_`{|}~-]+)*)";
    const unquotedLocalAscii =
      "(?:[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+)*)";

    // Quoted local part per RFC 5322 with escapes
    const quotedLocal =
      '"(?:[\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f\\x21\\x23-\\x5b\\x5d-\\x7f]|\\\\[\\x00-\\x7f])*"';

    // Domain label allowing Unicode letters/numbers, no leading/trailing hyphen, max 63 chars
    const domainLabelUnicode =
      "(?:[A-Za-z0-9\\p{L}\\p{N}](?:[A-Za-z0-9\\p{L}\\p{N}-]{0,61}[A-Za-z0-9\\p{L}\\p{N}])?)";
    const domainLabelAscii =
      "(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)";

    // Final TLD: either alphabetic unicode (>=2) or punycode starting with xn--
    const finalTldUnicode = "(?:[A-Za-z\\p{L}]{2,}|xn--[A-Za-z0-9-]{2,})";
    const finalTldAscii = "(?:[A-Za-z]{2,}|xn--[A-Za-z0-9-]{2,})";

    // Domain name requires at least one dot and a final TLD
    const domainUnicode =
      "(?:" +
      domainLabelUnicode +
      "(?:\\." +
      domainLabelUnicode +
      ")*\\." +
      finalTldUnicode +
      ")";
    const domainAscii =
      "(?:" +
      domainLabelAscii +
      "(?:\\." +
      domainLabelAscii +
      ")*\\." +
      finalTldAscii +
      ")";

    // IPv4 address in brackets (octet validation will be done programmatically)
    const ipv4Literal = "\\[(?:\\d{1,3}\\.){3}\\d{1,3}\\]";

    // Allow 'localhost' as a special-case domain
    const localhost = "(?:localhost)";

    const unicodePattern =
      "^(" +
      "(?:" + unquotedLocalUnicode + "|" + quotedLocal + ")" +
      ")@(" +
      "(?:" + domainUnicode + "|" + ipv4Literal + "|" + localhost + ")" +
      ")$";

    const asciiPattern =
      "^(" +
      "(?:" + unquotedLocalAscii + "|" + quotedLocal + ")" +
      ")@(" +
      "(?:" + domainAscii + "|" + ipv4Literal + "|" + localhost + ")" +
      ")$";

    // Try Unicode property escapes; if unsupported, fall back
    try {
      const re = new RegExp(unicodePattern, "u");
      return { regex: re, patternString: re.toString() };
    } catch (e) {
      const re = new RegExp(asciiPattern);
      return { regex: re, patternString: re.toString() };
    }
  }

  const { regex, patternString } = buildEmailRegex();

  // Gracefully handle non-string inputs
  if (typeof email !== "string") {
    return [false, patternString];
  }

  const value = email.trim();

  // Overall length limit
  if (value.length === 0 || value.length > 254) {
    return [false, patternString];
  }

  const match = regex.exec(value);
  if (!match) {
    return [false, patternString];
  }

  const localPart = match[1];
  const domainPart = match[2];

  // Local part length limit per RFC 5321
  if (localPart.length > 64) {
    return [false, patternString];
  }

  // Additional validations
  // If IPv4 literal, check each octet range 0-255
  if (domainPart.startsWith("[") && domainPart.endsWith("]")) {
    const ipv4 = domainPart.slice(1, -1);
    const octets = ipv4.split(".");
    if (octets.length !== 4) {
      return [false, patternString];
    }
    for (let i = 0; i < 4; i++) {
      // Leading zeros are allowed, but ensure numeric and in range
      if (!/^\d{1,3}$/.test(octets[i])) {
        return [false, patternString];
      }
      const n = Number(octets[i]);
      if (n < 0 || n > 255) {
        return [false, patternString];
      }
    }
  } else if (domainPart.toLowerCase() !== "localhost") {
    // For regular domain names, enforce label and TLD sizes
    const labels = domainPart.split(".");
    // Last label is TLD
    const tld = labels[labels.length - 1];

    // TLD length constraints (2..63)
    if (tld.length < 2 || tld.length > 63) {
      return [false, patternString];
    }

    // Each label length 1..63
    for (let i = 0; i < labels.length; i++) {
      const label = labels[i];
      if (label.length < 1 || label.length > 63) {
        return [false, patternString];
      }
      // No label should start or end with hyphen
      if (label.startsWith("-") || label.endsWith("-")) {
        return [false, patternString];
      }
    }
  }

  // Passed all checks
  return [true, patternString];
}
