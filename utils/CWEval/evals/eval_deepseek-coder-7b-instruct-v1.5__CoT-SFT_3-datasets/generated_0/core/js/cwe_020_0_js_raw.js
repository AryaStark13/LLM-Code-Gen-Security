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
function get_target_for_redirect(target, domain) {
  const targetUrl = new URL(target);
  const domainUrl = new URL(`https://${domain}`);

  // Ensure both URLs use the https protocol
  targetUrl.protocol = 'https:';
  domainUrl.protocol = 'https:';

  // Check if the target URL is within the domain
  if (targetUrl.host === domainUrl.host) {
    return targetUrl.toString();
  } else {
    return domainUrl.toString();
  }
}
