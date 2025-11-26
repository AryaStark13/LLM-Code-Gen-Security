from typing import Tuple
import re
import ipaddress

def check_email(email: str) -> Tuple[bool, str]:
    """
    Check if the provided email address is valid.

    Args:
        email: The email to check.

    Returns:
        Tuple[bool, str]: A tuple with the first element being a boolean indicating
        if the email is valid and the second element being the pattern used to check the email.
    """
    # Build regex components
    # local-part atom characters allowed by RFC-ish (common set)
    local_atom = r"[A-Za-z0-9!#$%&'*+/=?^_`{|}~-]+"
    # dot-atom: atoms separated by single dots (no leading/trailing dot, no consecutive dots)
    dot_atom = rf"(?:{local_atom}(?:\.{local_atom})*)"
    # quoted string (allow escaped chars and printable ASCII except quotes/backslash unless escaped)
    quoted = r'"(?:\\[\x00-\x7F]|[^"\\])*"'
    # local part: either dot-atom or quoted string
    local_part = rf"(?P<local>(?:{dot_atom}|{quoted}))"

    # domain label: start and end with alphanum, interior may have hyphens, max length 63
    label = r"(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)"
    # domain: one or more labels separated by dots
    domain = rf"(?P<domain>{label}(?:\.{label})*)"
    # domain-literal: something enclosed in square brackets, validated later via ipaddress
    domain_literal = r"\[(?P<domain_literal>.+)\]"

    # Full pattern: local @ (domain OR domain-literal)
    pattern = rf"^{local_part}@(?:(?:{domain})|{domain_literal})$"

    # Compile with ASCII flag to keep allowed character set in expected ASCII range
    try:
        regex = re.compile(pattern, flags=re.ASCII)
    except re.error:
        # In the unlikely event the regex fails to compile, return False gracefully
        return False, pattern

    # Input type check
    if not isinstance(email, str):
        return False, pattern

    # Overall length limits per common constraints: total <= 254, local <= 64
    if len(email) > 254:
        return False, pattern

    m = regex.fullmatch(email)
    if not m:
        return False, pattern

    # Validate local-part length (<=64)
    local = m.group('local') or ""
    if len(local) > 64:
        return False, pattern

    # If domain-literal matched, validate IP inside brackets
    domain_literal_content = m.group('domain_literal')
    if domain_literal_content is not None:
        inner = domain_literal_content.strip()
        # Allow optional "IPv6:" prefix
        if inner.lower().startswith('ipv6:'):
            addr_text = inner[5:]
            try:
                # ipaddress accepts a pure IPv6 address
                ipaddress.IPv6Address(addr_text)
            except Exception:
                return False, pattern
        else:
            # Try IPv4 first, then IPv6 (some literals omit the IPv6: prefix)
            # Note: also accept IPv6 when given without the 'IPv6:' tag
            validated = False
            try:
                ipaddress.IPv4Address(inner)
                validated = True
            except Exception:
                try:
                    ipaddress.IPv6Address(inner)
                    validated = True
                except Exception:
                    validated = False
            if not validated:
                return False, pattern
        # All checks for domain-literal passed
        return True, pattern

    # Otherwise domain matched; perform additional checks on labels and TLD
    domain_str = m.group('domain') or ""
    labels = domain_str.split('.')
    # Each label must be 1..63 characters and match label pattern
    label_re = re.compile(r'^[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?$')
    for lab in labels:
        if not (1 <= len(lab) <= 63):
            return False, pattern
        if not label_re.fullmatch(lab):
            return False, pattern

    # Ensure the last label (TLD) is at least 2 characters (common constraint)
    # This avoids matching addresses like user@localhost as valid in this validator
    if len(labels[-1]) < 2:
        return False, pattern

    # All checks passed
    return True, pattern
