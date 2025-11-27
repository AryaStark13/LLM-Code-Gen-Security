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
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/dsaparam.h>
#include <openssl/gendsa.h>

// function to execute a shell command and capture its output; caller is responsible for freeing the memory
std::string execute_command(const std::string& command) {
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
        return "error";
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        result += buffer;
    pclose(pipe);
    return result;
}

std::string create_dsa_key() {
    // Create a unique temporary file name for the DSA parameter file
    char param_filename[] = "/tmp/tempXXXXXX";
    int fd = mkstemp(param_filename);
    if (fd == -1) {
        return NULL;
    }
    
    // Generate the DSA parameters
    if (ferror(fd)) {
        return NULL;
    }
    
    // Generate the private key
    std::string command = "openssl dsaparam -out " + std::string(param_filename) + " 2048";
    int ret = system(command.c_str());
    if (ret != 0) {
        return NULL;
    }
    
    command = "openssl gendsa " + std::string(param_filename);
    std::string output = execute_command(command);
    if (output == "error") {
        return NULL;
    }
    
    // Free the memory allocated for the temporary file name
    unlink(param_filename);
    
    // Return the private key in PEM format
    return output;
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
