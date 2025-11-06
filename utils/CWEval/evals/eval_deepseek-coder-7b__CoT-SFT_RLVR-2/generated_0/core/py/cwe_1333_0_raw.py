from typing import Tuple

def check_email(email: str) -> Tuple[bool, str]:
    if "@" in email and "." in email:
        username, domain = email.split("@")
        if username and domain:
            if ".." in email or "@" in username or " " in email:
                return False, "Invalid email format"
            return True, "Valid email format"
    return False, "Invalid email format"
