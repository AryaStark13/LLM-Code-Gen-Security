import os
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding
from cryptography.hazmat.backends import default_backend

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    # Generate a random IV
    iv = os.urandom(16)

    # Create a cipher object using AES with the provided key and IV
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=default_backend())

    # Encrypt the data
    encryptor = cipher.encryptor()
    padded_data = padding.PKCS7(128).pad(data)
    encrypted_data = encryptor.update(padded_data) + encryptor.finalize()

    return encrypted_data, iv, key

# Example usage
key = os.urandom(32)  # Generate a random key
data = b"Hello, World!"  # Data to encrypt
encrypted_data, iv, key = encrypt_data(key, data)
print("Encrypted Data:", encrypted_data)
print("IV:", iv)
print("Key:", key)
