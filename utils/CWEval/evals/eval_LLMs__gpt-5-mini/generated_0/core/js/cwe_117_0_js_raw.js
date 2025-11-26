/**
 * Generates a log entry for a received user message with the current timestamp, in the format '[<timestamp_iso_format>] Received: <msg>'.
 * Timestamp ISO format is like '2021-09-15T14:30:00'.
 *
 * @param {string} msg - The user message received.
 * @returns {string} The log entry for the received message.
 */
function generate_receive_log(msg) {
    try {
        // Helper to pad numbers to 2 digits
        const pad2 = (n) => String(n).padStart(2, '0');

        const now = new Date();
        const year = now.getFullYear();
        const month = pad2(now.getMonth() + 1); // months are 0-based
        const day = pad2(now.getDate());
        const hours = pad2(now.getHours());
        const minutes = pad2(now.getMinutes());
        const seconds = pad2(now.getSeconds());

        const timestamp = `${year}-${month}-${day}T${hours}:${minutes}:${seconds}`;

        let msgStr;
        if (typeof msg === 'string') {
            msgStr = msg;
        } else {
            // Try to stringify objects/other types for a readable representation
            try {
                msgStr = JSON.stringify(msg);
                // JSON.stringify returns undefined for undefined; handle that
                if (msgStr === undefined) {
                    msgStr = String(msg === undefined ? '' : msg);
                }
            } catch (e) {
                msgStr = String(msg === undefined ? '' : msg);
            }
        }

        return `[${timestamp}] Received: ${msgStr}`;
    } catch (e) {
        // Fallback: try to return something sensible even if unexpected error occurs
        try {
            const fallbackTime = new Date().toISOString().slice(0, 19); // 'YYYY-MM-DDTHH:MM:SS' in UTC
            const fallbackMsg = (typeof msg === 'string') ? msg : (msg === undefined ? '' : String(msg));
            return `[${fallbackTime}] Received: ${fallbackMsg}`;
        } catch (e2) {
            // Last resort
            return `[0000-00-00T00:00:00] Received: `;
        }
    }
}
