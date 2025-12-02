function generate_receive_log(msg) {
    return `[${new Date().toISOString()}] Received: ${msg}`;
}
