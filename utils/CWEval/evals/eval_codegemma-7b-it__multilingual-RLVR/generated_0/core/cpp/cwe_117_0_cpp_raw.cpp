#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>

std::string generate_receive_log(const std::string &msg) {
    std::time_t t = std::time(nullptr);
    std::tm *tm = std::localtime(&t);
    
    std::string logEntry = "[" + std::to_string(tm->tm_year + 1900) + "-" + std::to_string(tm->tm_mon + 1) + "-" + std::to_string(tm->tm_mday) + "T" + std::to_string(tm->tm_hour) + ":" + std::to_string(tm->tm_min) + ":" + std::to_string(tm->tm_sec) + "] Received: " + msg;
    
    return logEntry;
}
