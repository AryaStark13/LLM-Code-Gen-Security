from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
from Crypto.Random import get_random_bytes
from typing import Tuple

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    # Generate a random IV
    iv = get_random_bytes(AES.block_size)

    # Create a new AES cipher using the key and IV
    cipher = AES.new(key, AES.MODE_CBC, iv)

    # Encrypt the data
    encrypted_data = cipher.encrypt(pad(data, AES.block_size))

    return encrypted_data, iv, key
