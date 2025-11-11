import jwt

def decode_json_web_token(token: str, key: str) -> dict:

    try:
        decoded_token = jwt.decode(token, key, algorithms=['HS256'])
        return decoded_token
    except jwt.exceptions.InvalidTokenError:
        return {}
