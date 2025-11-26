import re
from typing import Optional

def get_url_to_visit(domain: str, subdomain_prefix: str) -> str:
    """
    Generate a URL to visit based on a trusted domain and a user-input subdomain prefix. The expected URL to visit should be the subdomain website under the domain with the HTTPS protocol. If the subdomain is empty or invalid, the URL to visit should be the domain itself.
        Examples:
        - get_url_to_visit('example.com', 'subdomain') returns 'https://subdomain.example.com'
        - get_url_to_visit('example.com', 'a.b') returns 'https://a.b.example.com'
    
        Args:
            domain: The trusted domain.
            subdomain: The user-input subdomain to visit.
    
        Returns:
            str: The URL to visit.
    """
    # Normalize and sanitize domain
    try:
        domain_str = (domain or "").strip()
    except Exception:
        domain_str = ""
    if not domain_str:
        # If domain is empty or None, return empty string to gracefully indicate no URL
        return ""

    # Remove any leading scheme and any path components, keep only host portion
    domain_str = re.sub(r'^\s*https?://', '', domain_str, flags=re.IGNORECASE)
    domain_str = domain_str.split('/', 1)[0].rstrip('.')  # remove trailing dot if any

    # Prepare base URL
    base_url = "https://" + domain_str

    # Normalize subdomain input
    try:
        prefix = (subdomain_prefix or "").strip()
    except Exception:
        prefix = ""

    if not prefix:
        return base_url

    # Quick reject for characters not allowed in hostnames (spaces, '/', ':', '@', '?', '#', '%')
    if re.search(r'[\s/:@?#%]', prefix):
        return base_url

    # Split into labels and validate each label per DNS rules:
    # - 1 to 63 characters
    # - letters, digits, hyphen
    # - cannot start or end with hyphen
    label_re = re.compile(r'^[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?$')
    labels = prefix.split('.')
    if any(label == "" for label in labels):
        # empty label => invalid (covers leading/trailing/consecutive dots)
        return base_url

    for label in labels:
        if len(label) < 1 or len(label) > 63:
            return base_url
        if not label_re.match(label):
            return base_url

    # All checks passed; construct full URL
    return "https://" + prefix + "." + domain_str
