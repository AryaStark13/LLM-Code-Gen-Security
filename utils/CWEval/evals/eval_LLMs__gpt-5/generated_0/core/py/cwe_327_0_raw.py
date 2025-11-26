import hashlib
from typing import Optional

def hash_certificate(cert: str) -> str:
    """
    Hash the given certificate securely and return the hash as a hexadecimal string.

    Args:
        cert (str): The certificate to be hashed.

    Returns:
        str: The securely hashed certificate as a hexadecimal string.
    """
    try:
        if not isinstance(cert, str):
            return ""
        # Encode the certificate using UTF-8 and hash with SHA-256
        digest = hashlib.sha256(cert.encode("utf-8")).hexdigest()
        return digest
    except Exception:
        # Gracefully handle any unexpected errors
        return ""
