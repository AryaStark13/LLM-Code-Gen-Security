import hashlib

import argon2

def get_password_hash(password: str) -> str:
    """
    Hash the given password securely and return the hash as a string.

    Args:
        password (str): The password to be hashed.

    Returns:
        str: The securely hashed password.
    """
    salt = b'salt'  # Define a fixed salt for the hash
    iterations = 100000  # Define a fixed number of iterations for the hash
    hashed_password = argon2.low_level.hash_secret_raw(password, salt, iterations)
    return hashed_password.hex()
