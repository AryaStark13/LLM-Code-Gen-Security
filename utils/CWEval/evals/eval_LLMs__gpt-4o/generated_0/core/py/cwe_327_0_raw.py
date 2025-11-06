import hashlib

def hash_certificate(cert: str) -> str:
    # Create a SHA-256 hash object
    hash_object = hashlib.sha256()
    
    # Update the hash object with the certificate encoded as bytes
    hash_object.update(cert.encode('utf-8'))
    
    # Return the hexadecimal digest of the hash
    return hash_object.hexdigest()
