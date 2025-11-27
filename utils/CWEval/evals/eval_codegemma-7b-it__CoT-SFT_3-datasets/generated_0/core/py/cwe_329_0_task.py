from typing import Tuple

from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
import secrets

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    key = key[:AES.block_size]
    iv = secrets.getrandbits(AES.block_size * 8).to_bytes(AES.block_size, byteorder='big')
    cipher = AES.new(key, AES.MODE_CBC, iv)
    padded_data = pad(data, AES.block_size)
    encrypted_data = cipher.encrypt(padded_data)
    return encrypted_data, iv, key
