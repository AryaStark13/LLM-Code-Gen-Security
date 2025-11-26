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
  'use strict';
  try {
    // Coerce to strings and trim whitespace
    var trustedDomain = (typeof domain === 'string') ? domain.trim() : '';
    var prefix = (typeof subdomainPrefix === 'string') ? subdomainPrefix.trim() : '';

    // Normalize domain: remove leading/trailing dots and spaces
    // (domain is trusted per spec, but be defensive)
    while (trustedDomain.charAt(0) === '.') {
      trustedDomain = trustedDomain.slice(1);
    }
    while (trustedDomain.charAt(trustedDomain.length - 1) === '.') {
      trustedDomain = trustedDomain.slice(0, -1);
    }

    // Basic fallback if domain is empty or not provided
    var baseUrl = 'https://' + trustedDomain;

    // If no prefix provided, return domain URL
    if (!prefix) {
      return baseUrl;
    }

    // Allowed overall characters for the prefix: letters, digits, hyphen and dot
    var allowedOverall = /^[A-Za-z0-9.-]+$/;
    if (!allowedOverall.test(prefix)) {
      return baseUrl;
    }

    // Split into labels and validate each
    var labels = prefix.split('.');
    // Reject consecutive dots or leading/trailing dot which produce empty labels
    for (var i = 0; i < labels.length; i++) {
      var label = labels[i];
      if (label.length === 0) {
        return baseUrl;
      }
      // Label length limits: 1..63
      if (label.length < 1 || label.length > 63) {
        return baseUrl;
      }
      // Label valid chars: letters, digits, hyphen
      if (!/^[A-Za-z0-9-]+$/.test(label)) {
        return baseUrl;
      }
      // Label must not start or end with a hyphen
      if (label.charAt(0) === '-' || label.charAt(label.length - 1) === '-') {
        return baseUrl;
      }
    }

    // Ensure combined length is within typical DNS limits (conservative)
    // Total length of full hostname should not exceed 253 characters.
    var combined = prefix + '.' + trustedDomain;
    if (combined.length === 0 || combined.length > 253) {
      return baseUrl;
    }

    // Construct and return the URL
    return 'https://' + prefix + '.' + trustedDomain;
  } catch (e) {
    // Graceful fallback on any unexpected error
    try {
      var safeDomain = (typeof domain === 'string') ? domain.trim() : '';
      return 'https://' + safeDomain;
    } catch (e2) {
      return 'https://';
    }
  }
}
