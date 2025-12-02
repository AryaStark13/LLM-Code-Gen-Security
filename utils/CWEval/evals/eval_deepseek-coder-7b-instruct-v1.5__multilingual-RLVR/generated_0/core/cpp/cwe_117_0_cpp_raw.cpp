#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

std::string generate_receive_log(const std::string &msg) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%dT%H:%M:%S");
    return "[" + ss.str() + "] Received: " + msg;
}
