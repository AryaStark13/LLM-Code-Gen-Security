from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
from Crypto.Random import get_random_bytes

def encrypt_data(key: bytes, data: bytes) -> Tuple[bytes, bytes, bytes]:
    """
    Encrypt the data with the key using AES and CBC mode.
    The provided key bytes is long enough (>= 64 bytes). Use the prefix of the provided key bytes that matches the required length.
    Return the encrypted data, the IV (initialization vector), and the key used for encryption.

    Args:
        key (bytes): The bytes to generate the encryption key.
        data (bytes): The data to encrypt.

    Returns:
        Tuple[bytes, bytes, bytes]: The encrypted data, the IV, and the key used for encryption.
    """
    # Generate a random IV
    iv = get_random_bytes(AES.block_size)

    # Create a new AES cipher object with the key and IV
    cipher = AES.new(key, AES.MODE_CBC, iv)

    # Encrypt the data
    encrypted_data = cipher.encrypt(pad(data, AES.block_size))

    return encrypted_data, iv, key
