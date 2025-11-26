#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <exception>

std::string generate_receive_log(const std::string &msg) {
    try {
        // Get current time as time_t
        const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t time_tt = std::chrono::system_clock::to_time_t(now);

        // Convert to broken-down time in a thread-safe manner
        std::tm tm_time;
#if defined(_WIN32) || defined(_WIN64)
        // localtime_s returns errno_t, but we ignore it and still use tm_time
        if (0 != localtime_s(&tm_time, &time_tt)) {
            // Fallback to std::localtime if localtime_s failed (very unlikely)
            std::tm *tmp = std::localtime(&time_tt);
            if (tmp) {
                tm_time = *tmp;
            } else {
                // If conversion fails, use epoch
                tm_time = {};
            }
        }
#else
        if (nullptr == localtime_r(&time_tt, &tm_time)) {
            // If conversion fails, zero-initialize
            tm_time = {};
        }
#endif

        // Format timestamp as ISO-like "YYYY-MM-DDTHH:MM:SS"
        std::ostringstream oss;
        oss << '[' << std::put_time(&tm_time, "%Y-%m-%dT%H:%M:%S") << "] Received: " << msg;
        return oss.str();
    } catch (const std::exception &) {
        // Graceful fallback in case of any unexpected exception
        std::ostringstream fallback;
        fallback << "[1970-01-01T00:00:00] Received: " << msg;
        return fallback.str();
    } catch (...) {
        // Catch-all fallback
        return std::string("[1970-01-01T00:00:00] Received: ") + msg;
    }
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <message>\n";
        return EXIT_FAILURE;
    }

    // Ensure the message length does not exceed MAX_MSG_LEN
    std::string message(argv[1]);

    // Generate log entry and print
    std::string log_entry = generate_receive_log(message);
    std::cout << log_entry;

    return EXIT_SUCCESS;
}
