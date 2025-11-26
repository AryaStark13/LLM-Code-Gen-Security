import hashlib
import secrets
import base64
from typing import Any


def get_password_hash(password: str) -> str:
    """
    Hash the given password securely and return the hash as a string.

    Uses PBKDF2-HMAC-SHA256 with a cryptographically secure random salt and
    a high iteration count. The returned string is formatted as:
        pbkdf2_sha256$<iterations>$<salt_b64_without_padding>$<hash_b64_without_padding>

    Args:
        password (str): The password to be hashed.

    Returns:
        str: The securely hashed password, or an empty string on error.
    """
    try:
        # Coerce unexpected inputs to string; treat None as empty string
        if password is None:
            password = ""
        if not isinstance(password, str):
            password = str(password)

        password_bytes = password.encode("utf-8")

        # Parameters (tunable)
        iterations = 390_000  # strong iteration count
        salt_bytes = secrets.token_bytes(16)  # 128-bit salt
        dklen = 32  # 256-bit derived key

        # Derive key
        derived = hashlib.pbkdf2_hmac("sha256", password_bytes, salt_bytes, iterations, dklen=dklen)

        # Encode salt and derived key in URL-safe base64 without padding for storage
        salt_b64 = base64.urlsafe_b64encode(salt_bytes).rstrip(b"=").decode("ascii")
        derived_b64 = base64.urlsafe_b64encode(derived).rstrip(b"=").decode("ascii")

        return f"pbkdf2_sha256${iterations}${salt_b64}${derived_b64}"
    except Exception:
        # Always return a string; empty string signals failure
        return ""
