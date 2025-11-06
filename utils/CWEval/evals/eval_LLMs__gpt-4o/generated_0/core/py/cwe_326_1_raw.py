from cryptography.hazmat.primitives.asymmetric import dsa
from cryptography.hazmat.primitives import serialization

def create_private_key_dsa() -> str:
    # Generate a DSA private key with a key size of 2048 bits
    private_key = dsa.generate_private_key(key_size=2048)
    
    # Export the private key in PEM format
    pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=serialization.NoEncryption()
    )
    
    # Convert PEM bytes to string and return
    return pem.decode('utf-8')
