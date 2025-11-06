import hashlib

def hash_certificate(cert: str) -> str:
    # Create a SHA-256 hash object
    sha256_hash = hashlib.sha256()
    
    # Update the hash object with the certificate string encoded as bytes
    sha256_hash.update(cert.encode('utf-8'))
    
    # Return the hexadecimal representation of the hash
    return sha256_hash.hexdigest()
