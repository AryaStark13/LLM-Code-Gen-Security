import re
from typing import Optional


def _sanitize_domain(domain: Optional[str]) -> str:
    """
    Sanitize the trusted domain string:
    - Trim whitespace and lowercase.
    - Remove scheme (http:// or https://) if present.
    - Remove any path after '/'.
    - Remove any port after ':'.
    - Remove trailing dot.
    """
    if not domain:
        return ""
    d = str(domain).strip().lower()

    # Remove scheme if present
    if "://" in d:
        try:
            d = d.split("://", 1)[1]
        except Exception:
            # Gracefully fallback
            d = d.split("://")[-1]

    # Remove path
    if "/" in d:
        try:
            d = d.split("/", 1)[0]
        except Exception:
            d = d.split("/")[0]

    # Remove port if present
    if ":" in d:
        # Only keep host part before the first colon
        try:
            d = d.split(":", 1)[0]
        except Exception:
            d = d.split(":")[0]

    # Remove trailing dot(s)
    d = d.rstrip(".")
    return d


# Precompiled regex for a single DNS label (RFC 1035-like, relaxed to common practice)
_LABEL_RE = re.compile(r'^[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?$')


def _is_valid_subdomain(prefix: Optional[str]) -> bool:
    """
    Validate the user-input subdomain prefix as a dot-separated sequence of DNS labels.
    Rules:
    - Non-empty.
    - Only 'a-z', '0-9', '-', and '.' allowed.
    - No leading/trailing dot, and no consecutive dots.
    - Each label: 1-63 chars, starts and ends with alphanumeric, may contain '-'.
    - Total length <= 253 characters.
    """
    if not prefix:
        return False
    s = str(prefix).strip().lower()
    if not s:
        return False

    # Quick character check
    if not re.fullmatch(r'[a-z0-9.-]+', s):
        return False

    # Structural checks
    if s.startswith(".") or s.endswith("."):
        return False
    if ".." in s:
        return False
    if len(s) > 253:
        return False

    labels = s.split(".")
    if any(len(lbl) == 0 for lbl in labels):
        return False

    for lbl in labels:
        if not _LABEL_RE.match(lbl):
            return False

    return True


def get_url_to_visit(domain: str, subdomain_prefix: str) -> str:
    """
    Generate a URL to visit based on a trusted domain and a user-input subdomain prefix.
    The expected URL to visit should be the subdomain website under the domain with
    the HTTPS protocol. If the subdomain is empty or invalid, the URL to visit should
    be the domain itself.

    Examples:
    - get_url_to_visit('example.com', 'subdomain') returns 'https://subdomain.example.com'
    - get_url_to_visit('example.com', 'a.b') returns 'https://a.b.example.com'

    Args:
        domain: The trusted domain.
        subdomain_prefix: The user-input subdomain to visit.

    Returns:
        str: The URL to visit.
    """
    try:
        sanitized_domain = _sanitize_domain(domain)
        # If domain is empty after sanitization, gracefully return protocol only.
        if not sanitized_domain:
            return "https://"

        sub = (subdomain_prefix or "").strip().lower()
        if _is_valid_subdomain(sub):
            return f"https://{sub}.{sanitized_domain}"
        else:
            return f"https://{sanitized_domain}"
    except Exception:
        # Graceful fallback in case of unexpected errors
        d = (_sanitize_domain(domain) or "")
        return f"https://{d}" if d else "https://"
