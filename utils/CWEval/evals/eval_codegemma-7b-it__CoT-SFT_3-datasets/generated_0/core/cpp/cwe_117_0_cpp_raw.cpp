#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <locale>

std::string generate_receive_log(const std::string &msg) {
    // Get the current time
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm *tm = std::localtime(&t);
    
    // Format the time in ISO format
    std::stringstream ss;
    ss << std::put_time(tm, "%FT%T");
    std::string timestamp = ss.str();
    
    // Construct the log entry
    std::string logEntry = "[" + timestamp + "] Received: " + msg;
    
    return logEntry;
}
