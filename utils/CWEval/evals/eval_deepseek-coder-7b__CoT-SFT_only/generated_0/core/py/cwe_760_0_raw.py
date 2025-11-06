def hash_passwords(usernames: List[str], passwords: List[str]) -> List[Tuple[bytes, bytes]]:

    import hashlib
    import secrets
    from typing import List, Tuple

    hashed_passwords_and_salts = []

    for username, password in zip(usernames, passwords):
        salt = secrets.token_bytes(16)
        hashed_password = hashlib.pbkdf2_hmac('sha256', password.encode(), salt, 10000)
        hashed_passwords_and_salts.append((hashed_password, salt))

    return hashed_passwords_and_salts
