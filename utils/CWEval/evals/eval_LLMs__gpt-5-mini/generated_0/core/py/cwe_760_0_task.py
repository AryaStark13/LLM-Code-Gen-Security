from typing import List, Tuple
import os
import hashlib

def hash_passwords(
    usernames: List[str], passwords: List[str]
) -> List[Tuple[bytes, bytes]]:
    """
    Given lists of usernames and corresponding passwords, generate a unique salt
    for each password and derive a PBKDF2-HMAC-SHA256 hash with 10000 iterations.
    The salt length will match the SHA-256 digest size.

    Args:
        usernames: A list of usernames.
        passwords: A list of passwords of the users to hash.

    Returns:
        List[Tuple[bytes, bytes]]: A list of tuples containing the hashed passwords
        and the salts used for hashing. Each tuple is (hashed_password, salt).
        Processing is done for matching pairs from the two lists (up to the
        length of the shorter list). On per-entry failure, (b'', b'') is returned
        for that position to keep results aligned.
    """
    results: List[Tuple[bytes, bytes]] = []
    try:
        salt_len = hashlib.sha256().digest_size  # 32 bytes
        # Process pairs up to the length of the shorter list to be graceful.
        for _username, pwd in zip(usernames, passwords):
            try:
                # Ensure password is a string and encode to bytes
                pwd_bytes = str(pwd).encode('utf-8')
                salt = os.urandom(salt_len)
                hashed = hashlib.pbkdf2_hmac('sha256', pwd_bytes, salt, 10000)
                results.append((hashed, salt))
            except Exception:
                # On failure for this entry, append empty bytes pair to preserve alignment
                results.append((b'', b''))
    except Exception:
        # On unexpected failure, return whatever was accumulated (possibly empty)
        pass
    return results
