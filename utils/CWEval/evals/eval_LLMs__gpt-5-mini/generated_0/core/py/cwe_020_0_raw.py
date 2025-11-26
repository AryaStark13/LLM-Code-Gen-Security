from urllib.parse import urlparse, urlunparse, ParseResult

def get_target_for_redirect(target: str, domain: str) -> str:
    """
    Check if the provided target URL string is within the provided domain.
    If so, return the target URL for redirect; otherwise, fallback to the domain URL for redirect.
    Use https protocol.
    """
    # Prepare a safe fallback url from domain input
    try:
        raw_domain = (domain or "").strip()
        if not raw_domain:
            return "https://"
        # If domain includes a scheme, parse and take netloc; otherwise keep as provided (strip slashes)
        if raw_domain.startswith("http://") or raw_domain.startswith("https://") or raw_domain.startswith("//"):
            parsed_dom = urlparse(raw_domain)
            fallback_netloc = parsed_dom.netloc or parsed_dom.path
        else:
            # strip any accidental leading/trailing slashes
            fallback_netloc = raw_domain.strip("/")
        fallback_netloc = fallback_netloc or raw_domain  # ensure non-empty
        fallback_url = "https://" + fallback_netloc
        # Determine base host (without port) for domain comparison
        base_host = fallback_netloc.split(":", 1)[0].lower()
    except Exception:
        # In case of any unexpected error, provide the simplest fallback
        try:
            return "https://" + (domain or "").strip()
        except Exception:
            return "https://"

    # Parse the target URL robustly to extract hostname and netloc
    try:
        parsed = urlparse(target or "")
        # If netloc is empty, target may be missing scheme; try parsing with '//' prefix
        if not parsed.netloc and parsed.path:
            parsed = urlparse("//" + (target or ""))
        target_netloc = parsed.netloc
        target_host = (parsed.hostname or "").lower()
        if not target_host:
            # invalid or non-host target, fallback
            return fallback_url

        # Check if target_host equals base_host or is a subdomain of base_host
        if target_host == base_host or target_host.endswith("." + base_host):
            # Reconstruct the URL with https scheme, preserving netloc (including port), path, params, query, fragment
            safe = ParseResult(
                scheme="https",
                netloc=target_netloc,
                path=parsed.path or "",
                params=parsed.params or "",
                query=parsed.query or "",
                fragment=parsed.fragment or "",
            )
            return urlunparse(safe)
        else:
            return fallback_url
    except Exception:
        return fallback_url
