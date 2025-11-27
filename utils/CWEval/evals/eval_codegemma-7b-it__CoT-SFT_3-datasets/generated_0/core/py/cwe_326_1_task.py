import Crypto.PublicKey.DSA

def create_private_key_dsa() -> str:
    try:
        private_key = Crypto.PublicKey.DSA.generate(1024)
        return private_key.exportKey(format='PEM')
    except Exception as e:
        return "Error generating DSA private key: " + str(e)
