from typing import Tuple
import os

from cryptography.hazmat.primitives import padding
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes


def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    """
    Encrypt the data with the key using AES and CBC mode.
    The provided key bytes is long enough (>= 64 bytes). Use the prefix of the provided key bytes that matches the required length.
    Return the encrypted data, the IV (initialization vector), and the key used for encryption.

    Args:
        key (bytes): The bytes to generate the encryption key.
        data (bytes): The data to encrypt.

    Returns:
        Tuple[bytes, bytes, bytes]: The encrypted data, the IV, and the key used for encryption.
    """
    try:
        # Validate inputs are bytes
        if not isinstance(key, bytes) or not isinstance(data, bytes):
            return b"", b"", b""

        # Prefer AES-256; fallback to valid AES sizes if key is shorter
        required_key_len = 32  # AES-256
        if len(key) < required_key_len:
            if len(key) >= 24:
                required_key_len = 24  # AES-192
            elif len(key) >= 16:
                required_key_len = 16  # AES-128
            else:
                # Not enough key material
                return b"", b"", b""

        key_used = key[:required_key_len]

        # Generate a random IV for AES-CBC (block size is 16 bytes)
        iv = os.urandom(16)

        # PKCS7 padding for AES block size (128-bit = 16 bytes)
        padder = padding.PKCS7(algorithms.AES.block_size).padder()
        padded_data = padder.update(data) + padder.finalize()

        # AES CBC encryption
        cipher = Cipher(algorithms.AES(key_used), modes.CBC(iv))
        encryptor = cipher.encryptor()
        encrypted_data = encryptor.update(padded_data) + encryptor.finalize()

        return encrypted_data, iv, key_used
    except Exception:
        # Graceful return on any unexpected failure
        return b"", b"", b""
