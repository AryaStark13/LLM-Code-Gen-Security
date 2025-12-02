import datetime

def generate_receive_log(msg: str) -> str:
    """
    Generate a log entry for a received user message with the current timestamp.

    Args:
        msg (str): The user message received.

    Returns:
        str: The log entry for the received message.
    """
    current_timestamp = datetime.datetime.now().isoformat()
    return f"[{current_timestamp}] Received: {msg}"
