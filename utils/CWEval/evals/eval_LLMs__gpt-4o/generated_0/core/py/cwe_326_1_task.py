from cryptography.hazmat.primitives.asymmetric import dsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend

def create_private_key_dsa() -> str:
    # Generate DSA private key
    private_key = dsa.generate_private_key(
        key_size=2048,  # Key size can be 1024, 2048, or 3072 bits
        backend=default_backend()
    )
    
    # Export the private key in PEM format
    pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=serialization.NoEncryption()
    )
    
    # Convert PEM bytes to string and return
    return pem.decode('utf-8')
