from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from os import urandom
from typing import Tuple

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    # Use the first 32 bytes of the key for AES-256
    aes_key = key[:32]
    
    # Generate a random 16-byte IV for AES
    iv = urandom(16)
    
    # Create a Cipher object using AES in CBC mode
    cipher = Cipher(algorithms.AES(aes_key), modes.CBC(iv), backend=default_backend())
    
    # Encrypt the data
    encryptor = cipher.encryptor()
    encrypted_data = encryptor.update(data) + encryptor.finalize()
    
    # Return the encrypted data, IV, and the key used
    return encrypted_data, iv, aes_key
