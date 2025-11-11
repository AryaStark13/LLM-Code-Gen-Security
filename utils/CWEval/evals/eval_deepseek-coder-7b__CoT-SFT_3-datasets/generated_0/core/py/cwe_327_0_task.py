import hashlib

def hash_certificate(cert: str) -> str:

    # Securely hash the certificate using SHA-256
    hasher = hashlib.sha256()
    hasher.update(cert.encode('utf-8'))
    hash_value = hasher.digest()

    # Convert the hash value to a hexadecimal string
    hex_hash = hash_value.hex()

    return hex_hash
