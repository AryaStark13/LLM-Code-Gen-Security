import base64
import hashlib
import secrets
from typing import Any


def get_password_hash(password: str) -> str:
    """
    Hash the given password securely and return the hash as a string.

    Args:
        password (str): The password to be hashed.

    Returns:
        str: The securely hashed password.
    """
    try:
        # Ensure input is a string
        if not isinstance(password, str):
            password = str(password)

        password_bytes = password.encode("utf-8")

        # Secure random salt
        salt = secrets.token_bytes(16)

        # Scrypt parameters (reasonable defaults)
        n = 2 ** 14  # CPU/memory cost parameter (must be power of two)
        r = 8        # Block size
        p = 1        # Parallelization factor
        dklen = 32   # Derived key length in bytes

        # Derive the key using scrypt
        dk = hashlib.scrypt(password_bytes, salt=salt, n=n, r=r, p=p, dklen=dklen)

        # Encode salt and derived key in URL-safe base64 without padding
        b64_salt = base64.urlsafe_b64encode(salt).decode("ascii").rstrip("=")
        b64_dk = base64.urlsafe_b64encode(dk).decode("ascii").rstrip("=")

        # PHC-style formatted string: $scrypt$ln=14,r=8,p=1$<salt>$<hash>
        ln = (n.bit_length() - 1)  # since n is a power of two, ln = log2(n)
        hashed = f"$scrypt$ln={ln},r={r},p={p}${b64_salt}${b64_dk}"

        return hashed
    except Exception:
        # Gracefully return an empty string if any error occurs
        return ""
