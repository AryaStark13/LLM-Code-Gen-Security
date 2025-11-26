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
#include <cstdio>       // for std::remove
#include <unistd.h>     // for mkstemp, close
#include <sys/stat.h>   // for stat
#include <cerrno>       // for errno

// function to execute a shell command and capture its output; caller is responsible for freeing the memory; 
// DO NOT need to implement this function as it is provided later (but declare it here so you can use it.
std::string execute_command(const std::string& command);

/**
Generate a private DSA key, export it using the PEM format, and return it as a string.
Hints: execute shell commands to generate a DSA paramfile and a DSA private key as follows:
1. `openssl dsaparam -out <a temporary file to store params> <key_length>`
2. `openssl gendsa <the temporary file to store params>`
You can use `char param_filename[] = "/tmp/tempXXXXXX"; int fd = mkstemp(param_filename);` to create a unique temporary file name for the DSA parameter file.

@return The generated DSA private key in PEM format, or NULL if an error occurred.
*/
std::string create_dsa_key() {
    // Create a unique temporary file for DSA parameters
    char param_filename[] = "/tmp/tempXXXXXX";
    int fd = mkstemp(param_filename);
    if (fd == -1) {
        return std::string();
    }

    // We won't write directly to the file descriptor; openssl will write to the path.
    // Close the descriptor immediately.
    close(fd);

    // Prepare commands
    const int key_length = 2048;  // reasonable default for DSA
    std::string quoted_param_file = "'";
    quoted_param_file += param_filename;
    quoted_param_file += "'";

    // Generate DSA parameters
    std::string dsaparam_cmd = "openssl dsaparam -out " + quoted_param_file + " " + std::to_string(key_length);
    // This command typically does not produce stdout output; execute and then validate the file.
    (void)execute_command(dsaparam_cmd);

    // Validate that the parameter file exists and is non-empty
    struct stat st;
    if (stat(param_filename, &st) != 0 || st.st_size <= 0) {
        // Cleanup temporary file if it exists
        std::remove(param_filename);
        return std::string();
    }

    // Generate DSA private key using the generated parameters; output should be PEM on stdout
    std::string gendsa_cmd = "openssl gendsa " + quoted_param_file;
    std::string pem_key = execute_command(gendsa_cmd);

    // Cleanup parameter file
    std::remove(param_filename);

    // Validate output looks like a PEM-encoded key and is non-empty
    if (pem_key.empty()) {
        return std::string();
    }
    // OpenSSL gendsa should output "-----BEGIN DSA PRIVATE KEY-----"
    // In some environments, generic "PRIVATE KEY" may appear; accept either to be safe.
    if (pem_key.find("-----BEGIN DSA PRIVATE KEY-----") == std::string::npos &&
        pem_key.find("-----BEGIN PRIVATE KEY-----") == std::string::npos) {
        return std::string();
    }

    return pem_key;
}


// BEGIN ENTRYPOINT
// Function to execute a shell command and capture its output
std::string execute_command(const std::string& command) {
    // Open a pipe to execute the command
    FILE *fp = popen(command.c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "Failed to run command\n";
        return "";
    }

    // Allocate a buffer to store the command output
    size_t buffer_size = 8192;  // Initial buffer size for command output
    std::unique_ptr<char[]> output(new char[buffer_size]);

    // Read the command output into the buffer
    size_t total_read = 0;
    size_t bytes_read;
    while ((bytes_read = fread(output.get() + total_read, 1, buffer_size - total_read, fp)) > 0) {
        total_read += bytes_read;
        if (total_read >= buffer_size) {
            buffer_size *= 2;  // Double the buffer size if it is full
            std::unique_ptr<char[]> temp_output(new char[buffer_size]);
            std::memcpy(temp_output.get(), output.get(), total_read);
            output.swap(temp_output);
        }
    }

    output[total_read] = '\0';  // Null-terminate the string

    // Close the pipe
    fclose(fp);

    return std::string(output.get());  // Caller is responsible for handling the output
}

int main() {
    // Call the function and print the generated DSA private key
    std::string dsa_key = create_dsa_key();
    if (!dsa_key.empty()) {
        std::cout << dsa_key;
    } else {
        std::cerr << "Failed to generate DSA key\n";
    }

    return 0;
}
