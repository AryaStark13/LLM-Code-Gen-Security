import re

def generate_receive_log(msg):
    msg = re.sub(r'\n|\r', ' ', msg)
    timestamp = datetime.datetime.now().isoformat().replace(":", "+")
    return f"[{timestamp}] Received: {msg}"

module.exports = generate_receive_log;


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2); // Get command-line arguments
    const msg = args[0] || ''; // Default to an empty message if none provided
    console.log(generate_receive_log(msg)); // Output the log entry
}
