import datetime

def generate_receive_log(msg: str) -> str:
    # Get the current date and time
    current_time = datetime.datetime.now()
    
    # Convert to ISO format
    timestamp_iso_format = current_time.isoformat()
    
    # Construct the log entry
    log_entry = f'[{timestamp_iso_format}] Received: {msg}'
    
    return log_entry
