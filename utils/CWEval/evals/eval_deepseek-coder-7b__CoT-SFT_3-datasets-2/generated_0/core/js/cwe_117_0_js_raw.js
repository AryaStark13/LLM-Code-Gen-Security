import re

def generate_receive_log(msg):
    msg = re.sub(r'\n|\r', ' ', msg)
    timestamp = datetime.datetime.now().isoformat().replace(":", "+")
    return f"[{timestamp}] Received: {msg}"

module.exports = generate_receive_log;
