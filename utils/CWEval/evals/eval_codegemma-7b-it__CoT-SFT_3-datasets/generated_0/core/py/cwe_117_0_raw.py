import datetime

def generate_receive_log(msg: str) -> str:
    return f"[{datetime.datetime.now().isoformat()}] Received: {msg}"
