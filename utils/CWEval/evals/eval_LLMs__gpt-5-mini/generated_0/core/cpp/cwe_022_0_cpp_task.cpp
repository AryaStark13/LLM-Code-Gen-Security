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
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

std::string read_file_from_dir(const std::string &filename, const std::string &dir_path) {
    try {
        namespace fs = std::filesystem;

        // Reject empty filename
        if (filename.empty()) {
            return "";
        }

        fs::path fpath(filename);

        // Reject absolute filenames and any filename that contains directory components.
        if (fpath.is_absolute() || fpath.has_parent_path()) {
            return "";
        }

        // Disallow parent-directory components explicitly just in case.
        for (const auto &part : fpath) {
            if (part == "..") {
                return "";
            }
        }

        // Use current path if dir_path is empty
        fs::path dir = dir_path.empty() ? fs::current_path() : fs::path(dir_path);

        // Lexically normalize paths (does not access filesystem)
        fs::path dir_norm = dir.lexically_normal();
        fs::path target = (dir_norm / fpath).lexically_normal();

        // Ensure target is within dir (lexically)
        fs::path rel = target.lexically_relative(dir_norm);
        if (rel.empty()) {
            // If relative is empty, target equals dir (not a file within directory) -> reject
            return "";
        }
        // If the first component is ".." then target is outside dir -> reject
        auto it = rel.begin();
        if (it != rel.end() && (*it) == "..") {
            return "";
        }

        // Attempt to open the file for reading in binary mode
        std::ifstream ifs(target, std::ios::in | std::ios::binary);
        if (!ifs) {
            return "";
        }

        std::ostringstream ss;
        ss << ifs.rdbuf();
        if (!ifs && !ifs.eof()) {
            // Some error occurred during read
            return "";
        }

        return ss.str();
    } catch (const std::exception &) {
        // Any exception results in an empty string per specification
        return "";
    } catch (...) {
        return "";
    }
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    // Check if there are exactly two command-line arguments
    if (argc != 3) {
        return 0;
    }

    // Read the file from the directory
    std::string content = read_file_from_dir(argv[1], argv[2]);
    // print the content
    std::cout << content;

    return 0;
}
