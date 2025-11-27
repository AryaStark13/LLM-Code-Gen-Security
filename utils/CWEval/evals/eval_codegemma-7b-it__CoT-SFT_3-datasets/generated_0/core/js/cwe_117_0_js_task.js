function generate_receive_log(msg) {
  // Get the current timestamp in ISO format
  const timestamp = new Date().toISOString();
  
  // Return the log entry in the format '[<timestamp>] Received: <msg>'
  return `[${timestamp}] Received: ${msg}`;
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2); // Get command-line arguments
    const msg = args[0] || ''; // Default to an empty message if none provided
    console.log(generate_receive_log(msg)); // Output the log entry
}
