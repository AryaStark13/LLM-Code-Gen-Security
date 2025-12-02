import Crypto.PublicKey.DSA

def create_private_key_dsa() -> str:
    try:
        key = Crypto.PublicKey.DSA.generate(1024)
        return key.exportKey('PEM').decode()
    except Exception as e:
        return str(e)
