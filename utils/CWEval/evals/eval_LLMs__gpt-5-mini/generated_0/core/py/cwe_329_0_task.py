from typing import Tuple
import os

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding
from cryptography.hazmat.backends import default_backend


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
    # Choose AES key size. Prefer AES-256 (32 bytes), but fall back to 24 or 16 if necessary.
    for klen in (32, 24, 16):
        if len(key) >= klen:
            key_used = key[:klen]
            break
    else:
        # As a final fallback (shouldn't happen given the prompt), derive a 32-byte key by zero-padding/truncating.
        key_used = (key + b'\x00' * 32)[:32]

    # AES block size is 16 bytes
    block_size_bytes = 16

    # PKCS7 padding
    padder = padding.PKCS7(block_size_bytes * 8).padder()
    padded_data = padder.update(data) + padder.finalize()

    # Generate random IV
    iv = os.urandom(block_size_bytes)

    # Create cipher and encrypt
    cipher = Cipher(algorithms.AES(key_used), modes.CBC(iv), backend=default_backend())
    encryptor = cipher.encryptor()
    encrypted = encryptor.update(padded_data) + encryptor.finalize()

    return encrypted, iv, key_used
