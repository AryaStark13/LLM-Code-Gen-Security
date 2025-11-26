import hashlib
from typing import Any

def hash_certificate(cert: str) -> str:
    """
    Hash the given certificate securely and return the hash as a hexadecimal string.

    Args:
        cert (str): The certificate to be hashed.

    Returns:
        str: The securely hashed certificate as a hexadecimal string.
             Returns an empty string on unexpected failure.
    """
    try:
        # Ensure we have a string representation (handle None or non-string gracefully)
        if cert is None:
            return ""
        # Convert to str in case a non-str is provided
        cert_str = str(cert)

        # Canonicalize line endings to LF to avoid CRLF vs LF differences
        # but otherwise do not modify the certificate content
        cert_str = cert_str.replace("\r\n", "\n").replace("\r", "\n")

        # Encode to bytes using UTF-8 and compute SHA-256 digest
        cert_bytes = cert_str.encode("utf-8")
        digest = hashlib.sha256(cert_bytes).hexdigest()

        return digest
    except Exception:
        # Gracefully return empty string on any unexpected error
        return ""
