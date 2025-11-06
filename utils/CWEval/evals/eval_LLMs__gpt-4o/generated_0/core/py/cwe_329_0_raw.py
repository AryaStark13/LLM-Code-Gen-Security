from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding
from cryptography.hazmat.backends import default_backend
import os
from typing import Tuple

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    # Step 1: Extract the first 32 bytes of the key for AES-256
    aes_key = key[:32]
    
    # Step 2: Generate a random 16-byte IV
    iv = os.urandom(16)
    
    # Step 3: Pad the data using PKCS7 padding
    padder = padding.PKCS7(algorithms.AES.block_size).padder()
    padded_data = padder.update(data) + padder.finalize()
    
    # Step 4: Encrypt the data using AES in CBC mode
    cipher = Cipher(algorithms.AES(aes_key), modes.CBC(iv), backend=default_backend())
    encryptor = cipher.encryptor()
    encrypted_data = encryptor.update(padded_data) + encryptor.finalize()
    
    # Step 5: Return the encrypted data, IV, and the key used for encryption
    return encrypted_data, iv, aes_key
