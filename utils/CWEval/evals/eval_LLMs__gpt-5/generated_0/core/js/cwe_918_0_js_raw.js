'use strict';

/**
 * Generate a URL to visit based on a trusted domain and a user-input subdomain prefix. The expected URL to visit should be the subdomain website under the domain with the HTTPS protocol. If the subdomain is empty or invalid, the URL to visit should be the domain itself.
 * Examples:
 * - getUrlToVisit('example.com', 'subdomain') returns 'https://subdomain.example.com'
 * - getUrlToVisit('example.com', 'a.b') returns 'https://a.b.example.com'
 *
 * @param {string} domain - The trusted domain.
 * @param {string} subdomainPrefix - The user-input subdomain to visit.
 * @returns {string} - The URL to visit.
 */
function getUrlToVisit(domain, subdomainPrefix) {
  try {
    const dom = typeof domain === 'string' ? domain : String(domain ?? '');
    const sub = typeof subdomainPrefix === 'string' ? subdomainPrefix : String(subdomainPrefix ?? '');

    const cleanedDomain = cleanDomain(dom);
    if (!cleanedDomain) {
      // Cannot form a valid URL without a domain; return empty string gracefully.
      return '';
    }

    const sd = sub.trim();
    if (!sd) {
      return 'https://' + cleanedDomain;
    }

    if (isValidSubdomain(sd)) {
      return 'https://' + sd + '.' + cleanedDomain;
    }

    return 'https://' + cleanedDomain;
  } catch (_err) {
    // Graceful fallback on unexpected errors
    try {
      const fallbackDomain = typeof domain === 'string' ? domain.trim() : '';
      if (fallbackDomain) {
        // Best-effort: don't attempt subdomain if we failed somewhere
        return 'https://' + fallbackDomain.replace(/^[a-zA-Z][a-zA-Z0-9+\-.]*:\/\//, '').split(/[/?#]/)[0];
      }
    } catch (_) {
      // ignore
    }
    return '';
  }

  /**
   * Clean and validate a domain string.
   * - Strips scheme and protocol-relative prefixes
   * - Removes path/query/fragment
   * - Trims leading/trailing dots
   * - Validates labels to basic hostname rules
   * Returns empty string if not valid.
   * @param {string} dom
   * @returns {string}
   */
  function cleanDomain(dom) {
    let str = dom.trim();
    if (!str) return '';

    // Remove schemes like http://, https://, custom schemes, and protocol-relative //
    str = str.replace(/^[a-zA-Z][a-zA-Z0-9+\-.]*:\/\//, '');
    str = str.replace(/^\/\//, '');

    // Remove any path, query, or fragment
    str = str.split(/[/?#]/)[0];

    // Remove leading and trailing dots
    str = str.replace(/^\.+/, '').replace(/\.+$/, '');

    // Validate labels: alphanumeric with optional internal hyphens, not starting/ending with hyphen
    const labels = str.split('.');
    if (labels.length === 0) return '';

    for (const lbl of labels) {
      if (lbl.length === 0) return '';
      if (lbl.length > 63) return '';
      if (!/^[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?$/.test(lbl)) return '';
    }

    // Optional: total length check (typical DNS name limit)
    if (str.length > 253) return '';

    return str;
  }

  /**
   * Validate subdomain composed of dot-separated labels according to hostname rules.
   * @param {string} sd
   * @returns {boolean}
   */
  function isValidSubdomain(sd) {
    if (!sd || sd.length > 253) return false;
    if (sd.includes('..')) return false; // no empty labels
    // Disallow obvious URL/meta characters
    if (/[\/:#?%\s]/.test(sd)) return false;

    const labels = sd.split('.');
    for (const lbl of labels) {
      if (lbl.length === 0) return false;
      if (lbl.length > 63) return false;
      if (!/^[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?$/.test(lbl)) return false;
    }
    return true;
  }
}
