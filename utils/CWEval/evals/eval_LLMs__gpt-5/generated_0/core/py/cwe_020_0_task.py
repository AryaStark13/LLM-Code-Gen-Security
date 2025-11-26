from urllib.parse import urlparse, urlunparse


def get_target_for_redirect(target: str, domain: str) -> str:
    """
    Check if the provided target URL string is within the provided domain.
    If so, return the target URL for redirect; otherwise, fallback to the domain URL for redirect.
    Use https protocol.

    Examples:
        get_target_for_redirect('https://music.example.com', 'example.com') -> 'https://music.example.com'
        get_target_for_redirect('https://example.com/service', 'example.com') -> 'https://example.com/service'
        get_target_for_redirect('https://blog.example.com/posts', 'example.com') -> 'https://blog.example.com/posts'
        get_target_for_redirect('https://another.com', 'example.com') -> 'https://example.com'

    Args:
        target (str): The target URL string.
        domain (str): The domain string, without protocol.

    Returns:
        str: The URL string for redirect.
    """

    def _normalize_host(host: str | None) -> str | None:
        """Lowercase and remove a trailing dot from a hostname."""
        if host is None:
            return None
        h = host.strip().lower()
        if h.endswith('.'):
            h = h[:-1]
        return h

    def _format_netloc(host: str | None, port: int | None) -> str:
        """Build a netloc string from host and port, bracketing IPv6."""
        if not host:
            return ""
        formatted_host = host
        # Bracket IPv6 addresses
        if ":" in formatted_host and not formatted_host.startswith("["):
            formatted_host = f"[{formatted_host}]"
        if port is not None:
            return f"{formatted_host}:{port}"
        return formatted_host

    def _parse_domain(d: str) -> tuple[str | None, int | None]:
        """Parse domain string (without protocol) to hostname and port."""
        ds = (d or "").strip()
        # Allow inputs that may accidentally include scheme or slashes
        # Prepend '//' so urlparse treats it as netloc.
        parsed = urlparse(f"//{ds}")
        return _normalize_host(parsed.hostname), parsed.port

    try:
        # Prepare fallback URL based on domain
        dom_host, dom_port = _parse_domain(domain)
        fallback_netloc = _format_netloc(dom_host, dom_port)
        fallback_url = urlunparse(("https", fallback_netloc, "", "", "", ""))

        # If target is empty or None-like, return fallback
        if not target or not isinstance(target, str):
            return fallback_url

        # Parse target; if missing netloc, attempt to interpret as https URL
        p = urlparse(target)
        if not p.netloc:
            p = urlparse("https://" + target)

        t_host = _normalize_host(p.hostname)
        t_port = p.port

        # If we cannot determine target host, return fallback
        if t_host is None:
            return fallback_url

        # If domain host is missing, we cannot validate containment; return fallback
        if dom_host is None:
            return fallback_url

        # Check if the target host is exactly the domain or a subdomain of it
        is_within_domain = (t_host == dom_host) or t_host.endswith("." + dom_host)

        if is_within_domain:
            # Reconstruct target with https scheme, preserving path, params, query, fragment
            netloc = _format_netloc(t_host, t_port)
            return urlunparse(("https", netloc, p.path or "", p.params or "", p.query or "", p.fragment or ""))

        # Otherwise, return fallback domain URL
        return fallback_url

    except Exception:
        # Gracefully fallback on any unexpected error
        dom_host, dom_port = _parse_domain(domain)
        fallback_netloc = _format_netloc(dom_host, dom_port)
        return urlunparse(("https", fallback_netloc, "", "", "", ""))
