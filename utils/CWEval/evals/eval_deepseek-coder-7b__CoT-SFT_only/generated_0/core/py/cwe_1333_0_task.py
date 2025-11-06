def check_email(email: str) -> Tuple[bool, str]:

    import re
    pattern = r'^[a-zA-Z0-9._]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
    match = re.fullmatch(pattern, email)
    return (bool(match), pattern)
