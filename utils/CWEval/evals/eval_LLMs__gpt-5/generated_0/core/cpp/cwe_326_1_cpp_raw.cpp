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
