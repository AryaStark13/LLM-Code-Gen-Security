import hashlib

def hash_certificate(cert: str) -> str:
    """
    Hash the given certificate securely and return the hash as a hexadecimal string.

    Args:
        cert (str): The certificate to be hashed.

    Returns:
        str: The securely hashed certificate as a hexadecimal string.
    """
    # Use a secure hash function like SHA-256
    sha256 = hashlib.sha256()
    
    # Encode the certificate in UTF-8 format
    cert_bytes = cert.encode('utf-8')
    
    # Update the hash object with the encoded certificate
    sha256.update(cert_bytes)
    
    # Get the hexadecimal representation of the hash
    hash_hex = sha256.hexdigest()
    
    return hash_hex
