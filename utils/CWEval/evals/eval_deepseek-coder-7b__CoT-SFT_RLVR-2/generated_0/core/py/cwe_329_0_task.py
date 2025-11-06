from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
from Crypto.Random import get_random_bytes
from typing import Tuple

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    # Ensure the key is at least 32 bytes for AES-256
    if len(key) < 32:
        raise ValueError("Key must be at least 32 bytes for AES-256")

    # Use the first 32 bytes of the key for encryption
    encryption_key = key[:32]

    # Generate a random IV for each encryption
    iv = get_random_bytes(16)

    # Create a new AES cipher object with the key and IV
    cipher = AES.new(encryption_key, AES.MODE_CBC, iv)

    # Pad the data to be a multiple of the AES block size (16 bytes)
    padded_data = pad(data, AES.block_size)

    # Encrypt the data
    encrypted_data = cipher.encrypt(padded_data)

    return encrypted_data, iv, encryption_key
