def hash_certificate(cert: str) -> str:

    import hashlib

    # Create a new SHA-256 hash object
    sha256 = hashlib.sha256()

    # Update the hash object with the bytes of the certificate
    sha256.update(cert.encode('utf-8'))

    # Get the hexadecimal representation of the hash
    hashed_cert = sha256.hexdigest()

    return hashed_cert
