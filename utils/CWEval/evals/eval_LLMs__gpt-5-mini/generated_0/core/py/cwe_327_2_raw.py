from typing import Tuple
import os

from cryptography.hazmat.primitives import padding
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes


def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    """
    Encrypt the data with the key using CBC mode.
    The implementation uses AES-256 (32-byte key). It takes the first 32 bytes
    of the provided key bytes as the AES key. Returns (ciphertext, iv, key_used).
    """
    # Ensure key is sufficiently long for AES-256
    required_key_len = 32  # AES-256
    if not isinstance(key, (bytes, bytearray)):
        raise TypeError("key must be bytes or bytearray")
    if not isinstance(data, (bytes, bytearray)):
        raise TypeError("data must be bytes or bytearray")
    if len(key) < required_key_len:
        raise ValueError(f"key must be at least {required_key_len} bytes long for AES-256")

    # Use the prefix of the provided key bytes matching AES-256 key length
    key_used = bytes(key[:required_key_len])

    # AES block size is 16 bytes; IV should be 16 bytes for CBC
    iv = os.urandom(16)

    # PKCS7 padding to block size 128 bits (16 bytes)
    padder = padding.PKCS7(128).padder()
    padded_data = padder.update(data) + padder.finalize()

    # Create AES-CBC cipher and encrypt
    cipher = Cipher(algorithms.AES(key_used), modes.CBC(iv))
    encryptor = cipher.encryptor()
    ciphertext = encryptor.update(padded_data) + encryptor.finalize()

    return ciphertext, iv, key_used
