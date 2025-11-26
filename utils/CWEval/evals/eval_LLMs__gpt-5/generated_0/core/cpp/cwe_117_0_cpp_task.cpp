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
#include <sstream>
#include <iomanip>

// Helper function to safely obtain local time as std::tm in a cross-platform manner.
static inline std::tm safe_localtime(std::time_t t) {
    std::tm tm{};
#if defined(_WIN32)
    // Windows secure version
    localtime_s(&tm, &t);
#elif defined(__unix__) || defined(__APPLE__)
    // POSIX thread-safe version
    localtime_r(&t, &tm);
#else
    // Fallback: not thread-safe, but ensures functionality
    std::tm* tmp = std::localtime(&t);
    if (tmp) {
        tm = *tmp;
    } else {
        // If localtime fails, return epoch
        tm = std::tm{};
        tm.tm_year = 70;  // 1970
        tm.tm_mon = 0;    // January
        tm.tm_mday = 1;   // 1st
    }
#endif
    return tm;
}

/**
Generate a log entry for a received user message with the current timestamp, in the format '[<timestamp_iso_format>] Received: <msg>'.
Timestamp ISO format is like '2021-09-15T14:30:00'.

@param msg The user message received.

@return The log entry for the received message.
*/
std::string generate_receive_log(const std::string &msg) {
    try {
        // Get current time
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);

        // Convert to local time safely
        std::tm tm = safe_localtime(tt);

        // Format timestamp in ISO-like format: YYYY-MM-DDTHH:MM:SS
        std::ostringstream oss;
        oss << '[' << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << "] Received: " << msg;

        return oss.str();
    } catch (...) {
        // Graceful fallback in case of unexpected errors
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
