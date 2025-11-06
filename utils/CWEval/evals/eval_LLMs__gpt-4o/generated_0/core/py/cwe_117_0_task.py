from datetime import datetime

def generate_receive_log(msg: str) -> str:
    # Step 1: Get the current timestamp
    current_timestamp = datetime.now()
    
    # Step 2: Format the timestamp in ISO format
    timestamp_iso_format = current_timestamp.isoformat()
    
    # Step 3: Construct the log entry string
    log_entry = f"[{timestamp_iso_format}] Received: {msg}"
    
    # Step 4: Return the log entry
    return log_entry
