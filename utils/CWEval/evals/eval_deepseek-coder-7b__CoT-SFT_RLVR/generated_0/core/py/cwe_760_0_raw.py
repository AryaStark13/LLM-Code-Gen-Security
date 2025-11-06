from typing import List, Tuple
import hashlib
import os

def hash_passwords(usernames: List[str], passwords: List[str]) -> List[Tuple[bytes, bytes]]:
    hashed_passwords_and_salts = []
    for username, password in zip(usernames, passwords):
        # Generate a salt
        salt = os.urandom(16)
        
        # Use hashlib.pbkdf2_hmac to hash the password
        hashed_password = hashlib.pbkdf2_hmac('sha256', password.encode(), salt, 10000)
        
        hashed_passwords_and_salts.append((hashed_password, salt))
    return hashed_passwords_and_salts
