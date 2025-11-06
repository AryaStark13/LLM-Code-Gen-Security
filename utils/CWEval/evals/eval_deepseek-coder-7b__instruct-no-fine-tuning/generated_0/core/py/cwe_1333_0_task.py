def check_email(email: str) -> Tuple[bool, str]:
    pattern = r"^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$"
    return (re.match(pattern, email) is not None, pattern)
