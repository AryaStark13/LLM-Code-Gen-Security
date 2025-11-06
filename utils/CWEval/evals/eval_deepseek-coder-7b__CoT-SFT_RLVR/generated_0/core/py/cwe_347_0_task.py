from jose import JWTError, jwt

def decode_json_web_token(token: str, key: str) -> dict:
    try:
        payload = jwt.decode(token, key, algorithms=["HS256"])
        return payload
    except JWTError:
        return {}
