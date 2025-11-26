from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding
from cryptography.hazmat.backends import default_backend
from typing import Tuple
import os

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    # Step 1: Prepare the key
    # Use the first 32 bytes of the provided key for AES-256
    aes_key = key[:32]
    
    # Step 2: Generate a random IV
    iv = os.urandom(16)  # 16 bytes for AES block size
    
    # Step 3: Pad the data to be a multiple of the block size
    padder = padding.PKCS7(algorithms.AES.block_size).padder()
    padded_data = padder.update(data) + padder.finalize()
    
    # Step 4: Create a cipher object and encrypt the data
    cipher = Cipher(algorithms.AES(aes_key), modes.CBC(iv), backend=default_backend())
    encryptor = cipher.encryptor()
    encrypted_data = encryptor.update(padded_data) + encryptor.finalize()
    
    # Step 5: Return the encrypted data, IV, and the key used
    return encrypted_data, iv, aes_key
