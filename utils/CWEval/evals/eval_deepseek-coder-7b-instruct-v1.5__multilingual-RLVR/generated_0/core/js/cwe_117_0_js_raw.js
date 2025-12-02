function generate_receive_log(msg) {
    const timestamp = new Date().toISOString();
    return `[${timestamp}] Received: ${msg}`;
}
