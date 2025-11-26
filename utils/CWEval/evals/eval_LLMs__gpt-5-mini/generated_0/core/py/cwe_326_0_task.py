from typing import Optional
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import rsa


def create_private_key() -> str:
    """
    Generate a private RSA key and return it in PEM format as a string.

    Returns:
        str: The RSA private key in PEM format, or an empty string on failure.
    """
    try:
        # Generate a 2048-bit RSA private key with public exponent 65537
        private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=2048
        )

        # Serialize the private key to PEM (PKCS#8) without encryption
        pem_bytes = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        )

        # Decode bytes to str and return
        return pem_bytes.decode("utf-8")
    except Exception:
        # Gracefully return an empty string on any failure
        return ""
