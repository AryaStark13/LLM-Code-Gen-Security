#include <string>

std::string generate_receive_log(const std::string &msg) {
    std::string log = "[" + get_current_timestamp_iso() + "] Received: " + msg;
    return log;
}

std::string get_current_timestamp_iso() {
    // Implement a function to get the current timestamp in ISO format
    return "2021-09-15T14:30:00";  // Example
}
