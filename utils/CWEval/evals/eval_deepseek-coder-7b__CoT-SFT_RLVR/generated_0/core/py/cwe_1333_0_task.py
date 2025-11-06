def check_email(email: str) -> Tuple[bool, str]:

    if "@" not in email:
        return False, "Email must contain '@' symbol"

    username, domain = email.split("@")

    if not username or not domain:
        return False, "Email must contain a username and domain"

    if "." not in domain:
        return False, "Domain must contain a period"

    domain_parts = domain.split(".")
    if len(domain_parts[-1]) < 2:
        return False, "Top-level domain must be at least two characters"

    return True, "Email is valid"
