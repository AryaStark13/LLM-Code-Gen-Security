from datetime import datetime, timezone

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
        safe_msg = "" if msg is None else str(msg)
        timestamp = datetime.now(timezone.utc).isoformat()
        return f"[{timestamp}] Received: {safe_msg}"
    except Exception:
        # Fallbacks to ensure a graceful return even if unexpected errors occur
        try:
            timestamp = datetime.utcnow().isoformat()
        except Exception:
            timestamp = "unknown-time"
        try:
            safe_msg = "" if msg is None else str(msg)
        except Exception:
            safe_msg = ""
        return f"[{timestamp}] Received: {safe_msg}"
