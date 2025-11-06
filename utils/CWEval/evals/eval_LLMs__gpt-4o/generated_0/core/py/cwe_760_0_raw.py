import hashlib
import os
from typing import List, Tuple

def hash_passwords(usernames: List[str], passwords: List[str]) -> List[Tuple[bytes, bytes]]:
    hashed_passwords = []
    
    for password in passwords:
        # Generate a random salt of 32 bytes (same length as SHA-256 hash)
        salt = os.urandom(32)
        
        # Hash the password using pbkdf2_hmac with SHA-256, the generated salt, and 10000 iterations
        hashed_password = hashlib.pbkdf2_hmac('sha256', password.encode(), salt, 10000)
        
        # Append the tuple of hashed password and salt to the result list
        hashed_passwords.append((hashed_password, salt))
    
    return hashed_passwords
