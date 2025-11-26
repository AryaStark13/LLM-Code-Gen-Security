'use strict';

/**
 * Check if the provided target URL string is within the provided domain.
 * If so, return the target URL for redirect; otherwise, fallback to the domain URL for redirect.
 * Use https protocol.
 *
 * @param {string} target - The target URL string.
 * @param {string} domain - The domain string, without protocol.
 * @returns {string} The URL string for redirect.
 */
function get_target_for_redirect(target, domain) {
  try {
    // Normalize domain input
    const domainRaw = typeof domain === 'string' ? domain.trim() : '';
    const domainNormalized = domainRaw.toLowerCase();

    // Prepare fallback
    const fallback = 'https://' + domainNormalized;

    if (!domainNormalized) {
      // No domain provided; return a minimal https://
      return 'https://';
    }

    // Attempt to parse target URL
    let url = null;
    try {
      url = new URL(target);
    } catch (e) {
      // If target wasn't an absolute URL, try prefixing https://
      try {
        url = new URL('https://' + (typeof target === 'string' ? target.trim() : ''));
      } catch (e2) {
        // Invalid target: fallback
        return fallback;
      }
    }

    // Normalize hostname and extract port from domain if present
    const targetHostname = (url.hostname || '').toLowerCase();
    const targetPort = url.port || ''; // empty string if none

    // Handle domain that may include a port (e.g., example.com:8080)
    const domainParts = domainNormalized.split(':');
    const domainHost = domainParts[0];
    const domainPort = domainParts.length > 1 ? domainParts.slice(1).join(':') : '';

    // Determine whether target is within the domain (same host or subdomain)
    const isSameHost = targetHostname === domainHost;
    const isSubdomain =
      targetHostname.endsWith('.' + domainHost) && targetHostname.length > domainHost.length;

    const hostMatches = isSameHost || isSubdomain;

    // If domain included a port, ensure the ports match
    const portMatches = domainPort ? (targetPort === domainPort) : true;

    if (!hostMatches || !portMatches) {
      // Not within domain: fallback
      return fallback;
    }

    // Ensure https protocol in returned URL
    url.protocol = 'https:';

    // Build string and remove trailing slash when path is root and no search/hash (to match examples)
    let result = url.toString();

    if (url.pathname === '/' && (url.search === '' || url.search === null) && (url.hash === '' || url.hash === null)) {
      // Remove trailing slash if present
      if (result.endsWith('/')) {
        result = result.slice(0, -1);
      }
    }

    return result;
  } catch (err) {
    // Graceful fallback in case of unexpected errors
    try {
      const domainSafe = typeof domain === 'string' ? domain.trim() : '';
      return 'https://' + domainSafe;
    } catch (e) {
      return 'https://';
    }
  }
}
