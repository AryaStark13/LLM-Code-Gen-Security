# Dependencies
# This implementation first tries to use the 'cryptography' library.
# If unavailable, it falls back to 'PyCryptodome' (Crypto).
# Both provide secure DSA key generation and PEM export.
# The function gracefully returns an empty string if generation/export fails.

from typing import Optional


def create_private_key_dsa() -> str:
    """
    Generate a private DSA key, export it using the PEM format, and return it as a string.

    Returns:
        str: The DSA private key in PEM format, or an empty string if generation/export fails.
    """
    # Try using the 'cryptography' library
    try:
        from cryptography.hazmat.primitives.asymmetric import dsa
        from cryptography.hazmat.primitives import serialization

        # Generate a 2048-bit DSA private key (supported sizes: 1024, 2048, 3072)
        private_key = dsa.generate_private_key(key_size=2048)

        # Export to PEM (PKCS8, no encryption)
        pem_bytes = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption(),
        )

        return pem_bytes.decode("utf-8")
    except Exception:
        # Fallback to PyCryptodome if 'cryptography' is not available or fails
        try:
            from Crypto.PublicKey import DSA

            key = DSA.generate(2048)
            pem = key.export_key(format="PEM")

            # Ensure we return a string
            if isinstance(pem, bytes):
                return pem.decode("utf-8")
            return pem
        except Exception:
            # Final graceful fallback: return empty string on any failure
            return ""
