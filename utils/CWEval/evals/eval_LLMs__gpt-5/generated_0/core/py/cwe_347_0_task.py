from typing import Dict
import base64
import hmac
import hashlib
import json
import binascii


def decode_json_web_token(token: str, key: str) -> dict:
    """
    Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm.
    If the decoding fails, return an empty dictionary.

    Args:
        token (str): The JWT token to decode.
        key (str): The key used in encoding.

    Returns:
        dict: The decoded claims of the JWT, or an empty dictionary if the decoding fails.
    """
    try:
        if not isinstance(token, str) or not isinstance(key, str):
            return {}

        parts = token.split(".")
        if len(parts) != 3:
            return {}

        header_b64, payload_b64, signature_b64 = parts

        # Helper to add correct padding for base64url decode
        def _add_padding(b64url: str) -> str:
            padding_needed = (-len(b64url)) % 4
            if padding_needed:
                b64url += "=" * padding_needed
            return b64url

        # Decode header
        try:
            header_bytes = base64.urlsafe_b64decode(_add_padding(header_b64))
            header = json.loads(header_bytes.decode("utf-8"))
        except (binascii.Error, UnicodeDecodeError, json.JSONDecodeError):
            return {}

        # Ensure HS256 algorithm
        if not isinstance(header, dict) or header.get("alg") != "HS256":
            return {}

        # Decode payload
        try:
            payload_bytes = base64.urlsafe_b64decode(_add_padding(payload_b64))
            payload = json.loads(payload_bytes.decode("utf-8"))
        except (binascii.Error, UnicodeDecodeError, json.JSONDecodeError):
            return {}

        if not isinstance(payload, dict):
            return {}

        # Verify signature
        signing_input = (header_b64 + "." + payload_b64).encode("ascii")
        key_bytes = key.encode("utf-8")
        expected_sig = hmac.new(key_bytes, signing_input, hashlib.sha256).digest()
        expected_sig_b64 = base64.urlsafe_b64encode(expected_sig).decode("ascii").rstrip("=")

        # Normalize provided signature for comparison
        provided_sig_b64 = signature_b64.rstrip("=")

        if not hmac.compare_digest(provided_sig_b64, expected_sig_b64):
            return {}

        return payload

    except Exception:
        # Any unexpected error should result in a graceful empty dict return
        return {}
