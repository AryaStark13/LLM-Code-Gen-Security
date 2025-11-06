def get_password_hash(password: str) -> str:
    import hashlib
    password_hash = hashlib.sha256(password.encode()).hexdigest()
    return password_hash
