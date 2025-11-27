function generate_receive_log(msg) {
  // Get the current timestamp in ISO format
  const timestamp = new Date().toISOString();
  
  // Return the log entry in the format '[<timestamp>] Received: <msg>'
  return `[${timestamp}] Received: ${msg}`;
}
