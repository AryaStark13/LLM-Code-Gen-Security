#include <iostream>
#include <string>
#include <ctime>

std::string generate_receive_log(const std::string &msg) {
    std::time_t now = std::time(nullptr);
    std::string timestamp = std::ctime(&now);
    timestamp.erase(std::remove(timestamp.begin(), timestamp.end(), '\n'), timestamp.end());
    return "[" + timestamp + "] Received: " + msg;
}
