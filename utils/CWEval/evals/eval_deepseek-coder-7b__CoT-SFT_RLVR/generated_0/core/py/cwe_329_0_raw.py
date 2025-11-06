from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
from Crypto.Random import get_random_bytes
from typing import Tuple

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    key = key[:32]  # Use the first 32 bytes of the key
    iv = get_random_bytes(16)  # Generate a random IV
    cipher = AES.new(key, AES.MODE_CBC, iv)
    encrypted_data = cipher.encrypt(pad(data, AES.block_size))
    return encrypted_data, iv, key
