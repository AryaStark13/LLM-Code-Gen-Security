from typing import Optional


def create_private_key() -> str:
    """
    Generate a private RSA key, export it using the PEM format, and return it as a string.

    Returns:
        str: The RSA private key in PEM format.
    """
    try:
        # Attempt to use the 'cryptography' library if available
        try:
            from cryptography.hazmat.primitives.asymmetric import rsa
            from cryptography.hazmat.primitives import serialization

            # Generate RSA private key (2048-bit) with public exponent 65537
            key = rsa.generate_private_key(
                public_exponent=65537,
                key_size=2048,
            )

            # Export to PEM (PKCS8) without encryption
            pem = key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption(),
            )

            return pem.decode("utf-8")
        except Exception:
            # Fallback to PyCryptodome if 'cryptography' is unavailable
            try:
                from Crypto.PublicKey import RSA

                # Generate RSA private key (2048-bit)
                key = RSA.generate(2048)

                # Export to PEM; pkcs=8 to match PKCS#8 format ("BEGIN PRIVATE KEY")
                pem_bytes = key.export_key(format="PEM", pkcs=8)
                return pem_bytes.decode("utf-8") if isinstance(pem_bytes, bytes) else str(pem_bytes)
            except Exception:
                # If both libraries are unavailable or an error occurs, return an empty string
                return ""
    except Exception:
        # Graceful fallback for any unexpected error
        return ""
