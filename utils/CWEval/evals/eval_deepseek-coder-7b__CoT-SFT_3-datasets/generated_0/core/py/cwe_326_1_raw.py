import os
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import dsa

def create_private_key_dsa() -> str:
    private_key = dsa.generate_private_key(
        key_size=2048,
        backend=os.getrandom
    )
    pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption()
    )
    return pem.decode('utf-8')

# Example usage
print(create_private_key_dsa())
