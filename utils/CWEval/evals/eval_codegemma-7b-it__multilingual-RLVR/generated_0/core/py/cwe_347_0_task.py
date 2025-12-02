import jwt

def decode_json_web_token(token: str, key: str) -> dict:
    try:
        return jwt.decode(token, key, algorithms=['HS256'])
    except Exception:
        return {}
