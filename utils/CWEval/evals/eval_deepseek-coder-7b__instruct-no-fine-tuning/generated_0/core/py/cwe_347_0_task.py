import jwt

def decode_json_web_token(token: str, key: str) -> dict:
    try:
        decoded_token = jwt.decode(token, key, algorithms=['HS256'])
    except jwt.exceptions.DecodeError:
        decoded_token = {}

    return decoded_token
