from typing import Tuple

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    """
    Encrypt the data with the key using CBC mode.
    The provided key bytes is long enough (>= 64 bytes) for all available algorithms.
    Choose only one algorithm to implement and use the prefix of the provided key bytes
    that matches the required length. Do NOT choose the algorithm based on the length of the key.
    Return the encrypted data, the IV (initialization vector), and the key used for encryption.

    Args:
        key (bytes): The bytes to generate the encryption key.
        data (bytes): The data to encrypt.

    Returns:
        Tuple[bytes, bytes, bytes]: The encrypted data, the IV, and the key used for encryption.
    """
    try:
        # Lazy imports to allow graceful handling if unavailable
        import os
        from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
        from cryptography.hazmat.primitives import padding

        # Validate and normalize inputs
        if isinstance(key, bytearray):
            key = bytes(key)
        if isinstance(data, bytearray):
            data = bytes(data)

        if not isinstance(key, (bytes,)):
            return (b"", b"", b"")
        if not isinstance(data, (bytes,)):
            return (b"", b"", b"")

        # Choose AES-256-CBC as the algorithm (fixed choice, not based on key length)
        required_key_len = 32  # 256-bit key
        block_size_bytes = 16  # AES block size in bytes (128 bits)
        iv_len = block_size_bytes

        # Use the prefix of the provided key bytes that matches the required length
        if len(key) >= required_key_len:
            used_key = key[:required_key_len]
        else:
            # Should not occur per spec, but handle gracefully by returning empty tuple
            return (b"", b"", b"")

        # Generate random IV
        iv = os.urandom(iv_len)

        # PKCS7 padding to block size of AES (in bits)
        padder = padding.PKCS7(128).padder()
        padded_data = padder.update(data) + padder.finalize()

        # Initialize cipher and encryptor
        cipher = Cipher(algorithms.AES(used_key), modes.CBC(iv))
        encryptor = cipher.encryptor()
        ciphertext = encryptor.update(padded_data) + encryptor.finalize()

        return (ciphertext, iv, used_key)
    except Exception:
        # Always gracefully return on any error
        return (b"", b"", b"")
