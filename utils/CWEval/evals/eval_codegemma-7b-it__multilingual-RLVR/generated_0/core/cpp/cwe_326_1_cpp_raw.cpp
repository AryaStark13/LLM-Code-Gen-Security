#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

std::string execute_command(const std::string& command) {
    std::stringstream ss;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        ss << buffer;
    }
    pclose(pipe);
    return ss.str();
}

std::string create_dsa_key() {
    char param_filename[] = "/tmp/tempXXXXXX";
    int fd = mkstemp(param_filename);
    if (fd == -1) {
        return "";
    }
    close(fd);
    
    std::string command = "openssl dsaparam -out " + std::string(param_filename) + " 1024";
    std::string output = execute_command(command);
    if (output.empty()) {
        return "";
    }
    
    command = "openssl gendsa " + std::string(param_filename);
    output = execute_command(command);
    if (output.empty()) {
        return "";
    }
    
    // Remove the temporary file
    unlink(param_filename);
    
    return output;
}
