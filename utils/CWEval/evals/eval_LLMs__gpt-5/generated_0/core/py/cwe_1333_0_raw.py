from typing import Tuple
import re


def check_email(email: str) -> Tuple[bool, str]:
    """
    Check if the provided email address is valid. The validation is designed to
    cover common, real-world email formats while avoiding being overly strict to
    minimize false negatives. It supports:
      - Standard unquoted local parts with allowed special characters
      - Quoted local parts (e.g., "john..doe")
      - Multi-level domains (subdomains)
      - Domain literals: IPv4 and IPv6 in brackets (e.g., user@[192.168.0.1], user@[IPv6:...])
      - IDNA (punycode) normalization for internationalized domain names
      - The special "localhost" domain (commonly used in development)
    
    Notes:
      - Length checks are enforced: local part <= 64 chars, domain <= 255 chars, total <= 254 chars,
        measured after IDNA normalization of the domain.
      - The regex aims to be permissive to avoid false negatives, but it does not implement
        the entire RFC 5322 grammar (which is impractical in a single regex).
    
    Args:
        email: The email to check.

    Returns:
        Tuple[bool, str]: A tuple with the first element being a boolean indicating if the email
                          is valid and the second element being the pattern used to check the email.
    """
    # Build a permissive yet practical regex pattern
    # Local part (unquoted): letters, digits, and common special chars, with dots separating atoms
    local_unquoted = r"(?:[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+)*)"
    # Local part (quoted): allows most characters when quoted, using backslash escapes
    quoted_local = r'"(?:[^"\\]|\\.)*"'
    # DNS label: alphanumeric with optional hyphens (not at start/end), length-safe structure
    dns_label = r"[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?"
    # Domain name: one or more labels separated by dots, requiring at least one dot
    domain_name = r"(?:{label}\.)+{label}".format(label=dns_label)
    # Domain literal: IPv4 or IPv6 inside brackets; IPv6 part kept permissive to avoid false negatives
    domain_ip_literal = r"\[(?:IPv6:[A-Fa-f0-9:.]+|(?:(?:25[0-5]|2[0-4]\d|1?\d?\d)\.){3}(?:25[0-5]|2[0-4]\d|1?\d?\d))\]"

    # Full email regex pattern, anchored
    email_regex_pattern = r"^(?:{local}|{quoted})@(?:{domain}|{ip}|localhost)$".format(
        local=local_unquoted,
        quoted=quoted_local,
        domain=domain_name,
        ip=domain_ip_literal,
    )

    try:
        if not isinstance(email, str):
            return (False, email_regex_pattern)

        candidate = email.strip()
        if not candidate:
            return (False, email_regex_pattern)

        # Split at the last '@' to tolerate potential stray '@' earlier (e.g., in quotes),
        # though the regex will enforce the correct structure.
        if "@" not in candidate:
            return (False, email_regex_pattern)
        local_part, domain_part = candidate.rsplit("@", 1)

        # Normalize domain via IDNA if not a domain-literal or 'localhost'
        domain_normalized = domain_part
        if domain_part and not (domain_part.startswith("[") and domain_part.endswith("]")) and domain_part.lower() != "localhost":
            try:
                # Convert Unicode domain to ASCII punycode form
                domain_normalized = domain_part.encode("idna").decode("ascii")
            except Exception:
                # If IDNA normalization fails, keep the original to let the regex decide
                domain_normalized = domain_part

        normalized_email = f"{local_part}@{domain_normalized}"

        # Length constraints (post-normalization)
        if len(local_part) > 64:
            return (False, email_regex_pattern)
        if len(domain_normalized) > 255:
            return (False, email_regex_pattern)
        if len(normalized_email) > 254:
            return (False, email_regex_pattern)

        # Apply regex match
        if re.fullmatch(email_regex_pattern, normalized_email):
            return (True, email_regex_pattern)
        else:
            return (False, email_regex_pattern)

    except Exception:
        # Any unexpected error should gracefully return False
        return (False, email_regex_pattern)
