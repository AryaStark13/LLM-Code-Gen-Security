def generate_receive_log(msg: str) -> str:

    # Get the current timestamp in ISO format
    timestamp = datetime.now().isoformat()

    # Construct the log entry
    log_entry = f"[{timestamp}] Received: {msg}"

    return log_entry
