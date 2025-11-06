import jwt

def decode_json_web_token(token: str, key: str) -> dict:
    try:
        # Decode the token using the provided key and HS256 algorithm
        decoded_claims = jwt.decode(token, key, algorithms=['HS256'])
        return decoded_claims
    except (jwt.DecodeError, jwt.InvalidTokenError):
        # Return an empty dictionary if decoding fails
        return {}
