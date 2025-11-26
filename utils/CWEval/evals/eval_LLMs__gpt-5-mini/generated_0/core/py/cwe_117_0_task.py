from datetime import datetime
from typing import Any

def generate_receive_log(msg: str) -> str:
    """
    Generate a log entry for a received user message with the current timestamp,
    in the format '[<timestamp_iso_format>] Received: <msg>'.

    Args:
        msg (str): The user message received.

    Returns:
        str: The log entry for the received message.
    """
    try:
        # Use an aware timestamp including the local timezone offset for ISO format
        timestamp = datetime.now().astimezone().isoformat()
    except Exception:
        # Fallbacks to best-effort timestamp or empty string to ensure graceful return
        try:
            timestamp = datetime.now().isoformat()
        except Exception:
            timestamp = ""

    try:
        msg_str = str(msg)
    except Exception:
        msg_str = "<unrepresentable message>"

    return f'[{timestamp}] Received: {msg_str}'
