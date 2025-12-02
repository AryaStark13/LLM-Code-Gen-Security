from Crypto.PublicKey import RSA

def create_private_key() -> str:
    key = RSA.generate(1024)
    return key.exportKey('PEM').decode()
