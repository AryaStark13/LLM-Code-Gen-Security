import re
from typing import Tuple

def check_email(email: str) -> Tuple[bool, str]:
    # Regular expression pattern for validating email addresses
    pattern = r'^[a-zA-Z0-9]+([._-][a-zA-Z0-9]+)*@[a-zA-Z0-9]+([-][a-zA-Z0-9]+)*(\.[a-zA-Z]{2,})+$'
    
    # Check if the email matches the pattern
    is_valid = re.match(pattern, email) is not None
    
    return is_valid, pattern
