import base64
import json
import hashlib
import hmac
from typing import Dict


def decode_json_web_token(token: str, key: str) -> dict:
    """
    Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty dictionary.

        Args:
            token (str): The JWT token to decode.
            key (str): The key used in encoding.

        Returns:
            dict: The decoded claims of the JWT, or an empty dictionary if the decoding fails.
    """
    try:
        if not isinstance(token, str) or not isinstance(key, str):
            return {}

        parts = token.split('.')
        if len(parts) != 3:
            return {}

        header_b64, payload_b64, signature_b64 = parts

        def b64url_decode(input_str: str) -> bytes:
            # Add padding if necessary
            rem = len(input_str) % 4
            if rem:
                input_str += '=' * (4 - rem)
            return base64.urlsafe_b64decode(input_str.encode('ascii'))

        # Decode header and payload
        try:
            header_bytes = b64url_decode(header_b64)
            payload_bytes = b64url_decode(payload_b64)
        except (TypeError, base64.binascii.Error):
            return {}

        try:
            header = json.loads(header_bytes.decode('utf-8'))
            payload = json.loads(payload_bytes.decode('utf-8'))
        except (ValueError, UnicodeDecodeError):
            return {}

        # Ensure algorithm is HS256
        alg = header.get('alg') if isinstance(header, dict) else None
        if alg != 'HS256':
            return {}

        # Verify signature
        signing_input = (header_b64 + '.' + payload_b64).encode('ascii')
        key_bytes = key.encode('utf-8')
        dig = hmac.new(key_bytes, signing_input, hashlib.sha256).digest()
        expected_sig = base64.urlsafe_b64encode(dig).decode('ascii').rstrip('=')

        # Normalize provided signature (strip padding)
        provided_sig = signature_b64
        if '=' in provided_sig:
            # remove any padding for comparison
            provided_sig = provided_sig.rstrip('=')

        if not hmac.compare_digest(expected_sig, provided_sig):
            return {}

        # Ensure payload is a dict
        if isinstance(payload, dict):
            return payload
        else:
            return {}
    except Exception:
        # Any unexpected error -> return empty dict gracefully
        return {}
