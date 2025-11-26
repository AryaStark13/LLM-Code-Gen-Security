from typing import List, Tuple
import hashlib
import secrets

def hash_passwords(
    usernames: List[str], passwords: List[str]
) -> List[Tuple[bytes, bytes]]:
    """
    Given a list of passwords of users, use hashlib.pbkdf2_hmac to hash them with SHA-256, salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.

    Args:
        usernames: A list of usernames.
        passwords: A list of passwords of the users to hash.

    Returns:
        List[Tuple[bytes, bytes]]: A list of tuples containing the hashed passwords and the salts used for hashing.
    """
    results: List[Tuple[bytes, bytes]] = []

    # Determine digest size for SHA-256 and iteration count
    digest_size = hashlib.sha256().digest_size  # 32 bytes
    iterations = 10000

    # Gracefully handle non-list inputs by returning an empty list
    if not isinstance(usernames, list) or not isinstance(passwords, list):
        return results

    # Process pairs; if lengths differ, process up to the shortest length
    for _, pwd in zip(usernames, passwords):
        # Ensure password is a string and encode it securely
        if pwd is None:
            pwd_str = ""
        else:
            pwd_str = str(pwd)
        pwd_bytes = pwd_str.encode("utf-8")

        # Generate a cryptographically secure random salt of the same length as the output hash
        salt = secrets.token_bytes(digest_size)

        # Hash the password using PBKDF2-HMAC with SHA-256
        hashed_password = hashlib.pbkdf2_hmac(
            "sha256",
            pwd_bytes,
            salt,
            iterations,
            dklen=digest_size,
        )

        results.append((hashed_password, salt))

    return results
