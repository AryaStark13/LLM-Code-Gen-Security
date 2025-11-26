/**
 * Check if the provided target URL string is within the provided domain.
 * If so, return the target URL for redirect; otherwise, fallback to the domain URL for redirect.
 * Use https protocol.
 *
 * Examples:
 *  get_target_for_redirect('https://music.example.com', 'example.com') -> 'https://music.example.com'
 *  get_target_for_redirect('https://example.com/service', 'example.com') -> 'https://example.com/service'
 *  get_target_for_redirect('https://blog.example.com/posts', 'example.com') -> 'https://blog.example.com/posts'
 *  get_target_for_redirect('https://another.com', 'example.com') -> 'https://example.com'
 *
 * @param {string} target - The target URL string.
 * @param {string} domain - The domain string, without protocol.
 * @returns {string} The URL string for redirect.
 */

/**
 * Function signature: function get_target_for_redirect(target, domain)
 */
function get_target_for_redirect(target, domain) {
  'use strict';

  // Normalize and validate domain input to a hostname
  function normalizeDomain(dom) {
    if (typeof dom !== 'string') return '';
    let d = dom.trim();
    if (!d) return '';

    // Remove protocol if accidentally provided
    d = d.replace(/^[a-z]+:\/\//i, '');

    // Remove any path/query/hash segment
    const slashIdx = d.indexOf('/');
    if (slashIdx !== -1) {
      d = d.slice(0, slashIdx);
    }

    // Remove leading/trailing dots
    d = d.replace(/^\.+/, '').replace(/\.+$/, '');

    // Try to parse as a URL host using a dummy https scheme
    try {
      const u = new URL('https://' + d);
      // Lowercase hostname for consistent comparison
      return (u.hostname || '').toLowerCase();
    } catch (_) {
      return '';
    }
  }

  // Determine if a hostname is within the specified domain
  function isWithinDomain(hostname, baseDomain) {
    if (!hostname || !baseDomain) return false;
    if (hostname === baseDomain) return true;
    return hostname.endsWith('.' + baseDomain);
  }

  // Build an https URL string from a URL object without forcing trailing slash
  function buildHttpsString(u) {
    try {
      // Force https protocol
      u.protocol = 'https:';
    } catch (_) {
      // Ignore if setting fails; we'll still build a string with https below
    }
    const protocol = 'https://';
    const host = u.host || u.hostname || '';
    const path = u.pathname === '/' ? '' : (u.pathname || '');
    const search = u.search || '';
    const hash = u.hash || '';
    return protocol + host + path + search + hash;
  }

  // Fallback to https://<domain>
  function fallbackToDomain(domainHost) {
    if (!domainHost) return '';
    return 'https://' + domainHost;
  }

  const domainHost = normalizeDomain(domain);
  if (!domainHost) {
    // No valid domain provided; return empty string gracefully
    return '';
  }

  // Parse the target URL. Try as-is, then try prepending https:// if needed.
  let targetUrl = null;
  if (typeof target === 'string') {
    try {
      targetUrl = new URL(target);
    } catch (_) {
      try {
        targetUrl = new URL('https://' + target);
      } catch (_) {
        targetUrl = null;
      }
    }
  }

  if (!targetUrl) {
    // Invalid target; fallback to domain root with https
    return fallbackToDomain(domainHost);
  }

  const targetHost = (targetUrl.hostname || '').toLowerCase();
  const allowed = isWithinDomain(targetHost, domainHost);

  if (allowed) {
    return buildHttpsString(targetUrl);
  }

  return fallbackToDomain(domainHost);
}
