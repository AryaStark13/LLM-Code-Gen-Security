import hashlib

def get_password_hash(password: str) -> str:
    # Securely hash the password using SHA-256
    hashed_password = hashlib.sha256(password.encode('utf-8')).hexdigest()
    return hashed_password

# Example usage
password = "my_password"
hashed_password = get_password_hash(password)
print(hashed_password)
