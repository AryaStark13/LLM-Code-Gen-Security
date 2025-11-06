import os
import hashlib
from typing import List, Tuple

def hash_passwords(usernames: List[str], passwords: List[str]) -> List[Tuple[bytes, bytes]]:
    hashed_passwords_and_salts = []
    for password in passwords:
        salt = os.urandom(16)  # Generate a random salt of 16 bytes
        hashed_password = hashlib.pbkdf2_hmac('sha256', password.encode(), salt, 10000)  # Hash the password
        hashed_passwords_and_salts.append((hashed_password, salt))  # Append the hashed password and the salt
    return hashed_passwords_and_salts
