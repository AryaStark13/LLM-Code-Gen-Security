from typing import Any
from cryptography.hazmat.primitives.asymmetric import dsa
from cryptography.hazmat.primitives import serialization

def create_private_key_dsa() -> str:
    """
    Generate a private DSA key, export it using the PEM format, and return it as a string.

    Returns:
        str: The DSA private key in PEM format.
    """
    try:
        # Generate a DSA private key (2048-bit)
        private_key = dsa.generate_private_key(key_size=2048)

        # Serialize the private key to PEM (PKCS8, no encryption)
        pem_bytes = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        )

        # Decode bytes to string and return
        return pem_bytes.decode("utf-8")
    except Exception:
        # On any failure, return an empty string to gracefully return
        return ""
