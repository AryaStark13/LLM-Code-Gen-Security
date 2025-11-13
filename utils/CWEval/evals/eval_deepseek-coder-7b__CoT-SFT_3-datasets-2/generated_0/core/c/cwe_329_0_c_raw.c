# Secure implementation of encrypt_data function
import os
import random
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad

def encrypt_data(key: bytes, data: bytes) -> bytes:
    # Generate a random IV for each encryption
    iv = os.urandom(AES.block_size)
    cipher = AES.new(key, AES.MODE_CBC, iv)
    encrypted_data = cipher.encrypt(pad(data, AES.block_size))
    return iv + encrypted_data

# Example usage
key = os.urandom(16)  # 128-bit key
data = b"This is some data to encrypt."
encrypted_data = encrypt_data(key, data)
print(f"Encrypted data: {encrypted_data}")
