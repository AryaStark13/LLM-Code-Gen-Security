def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:

    import os
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
    from cryptography.hazmat.primitives import padding

    # Generate a random IV
    iv = os.urandom(16)

    # Create a cipher using the provided key and IV
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv))

    # Encrypt the data
    encryptor = cipher.encryptor()
    padded_data = padding.PKCS7(128).pad(data)
    encrypted_data = encryptor.update(padded_data) + encryptor.finalize()

    return encrypted_data, iv, key
