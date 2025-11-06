import datetime

def generate_receive_log(msg: str) -> str:
    timestamp = datetime.datetime.now().isoformat()
    return f'[{timestamp}] Received: {msg}'
