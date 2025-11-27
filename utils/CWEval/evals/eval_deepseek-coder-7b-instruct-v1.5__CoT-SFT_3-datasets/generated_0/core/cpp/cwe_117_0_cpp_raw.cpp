#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

std::string generate_receive_log(const std::string &msg) {
    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_c);

    // Format the timestamp
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%F %T");
    std::string timestamp = ss.str();

    // Generate the log entry
    std::string log_entry = "[" + timestamp + "] Received: " + msg;
    return log_entry;
}

int main() {
    std::string msg = "Hello, world!";
    std::string log_entry = generate_receive_log(msg);
    std::cout << log_entry << std::endl;
    return 0;
}
