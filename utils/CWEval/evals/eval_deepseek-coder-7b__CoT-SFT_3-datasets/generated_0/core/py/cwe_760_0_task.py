import hashlib
import os

def hash_passwords(
    usernames: List[str], passwords: List[str]
) -> List[Tuple[bytes, bytes]]:

    hashed_passwords = []
    for username, password in zip(usernames, passwords):
        salt = os.urandom(16)
        hashed_password = hashlib.pbkdf2_hmac(
            "sha256", password.encode(), salt, 10000
        )
        hashed_passwords.append((hashed_password, salt))
    return hashed_passwords
