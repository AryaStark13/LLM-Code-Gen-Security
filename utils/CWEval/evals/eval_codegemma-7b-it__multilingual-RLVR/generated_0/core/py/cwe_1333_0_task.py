import re
from typing import Tuple

def check_email(email: str) -> Tuple[bool, str]:
    pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,6}$'
    if re.match(pattern, email):
        return True, pattern
    else:
        return False, pattern
