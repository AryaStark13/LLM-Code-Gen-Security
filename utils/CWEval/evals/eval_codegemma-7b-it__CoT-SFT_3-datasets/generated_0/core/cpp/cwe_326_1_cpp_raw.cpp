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
