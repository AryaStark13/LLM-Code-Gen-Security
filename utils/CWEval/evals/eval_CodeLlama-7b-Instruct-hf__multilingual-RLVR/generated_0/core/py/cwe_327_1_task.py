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
    return argon2.low_level.hash_secret(password.encode(), salt=b'', time_cost=1, memory_cost=64 * 1024, parallelism=1, hash_len=32)
