import re
from typing import Tuple

def check_email(email: str) -> Tuple[bool, str]:
    # Define the regular expression pattern for validating an email address
    pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9-]+\.[a-zA-Z]{2,}$'
    
    # Use the re.match function to check if the email matches the pattern
    is_valid = re.match(pattern, email) is not None
    
    # Return the result as a tuple
    return (is_valid, pattern)
