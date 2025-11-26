#include <string>
#include <unistd.h>     // mkstemp, close, unlink
#include <sys/stat.h>   // stat
#include <fcntl.h>      // O_* constants (not strictly needed)
#include <cerrno>
#include <cstring>      // strerror
#include <stdexcept>

// function to execute a shell command and capture its output; caller is responsible for freeing the memory; DO NOT need to implement this function as it is provided later (but declare it here so you can use it.
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
    const int KEY_LENGTH = 2048;
    // Template for mkstemp
    char tmp_template[] = "/tmp/dsaparamXXXXXX";
    int fd = mkstemp(tmp_template);
    if (fd == -1) {
        // Failed to create temp file
        return std::string();
    }

    // Close the file descriptor; openssl dsaparam will write to the path
    close(fd);

    std::string param_path(tmp_template);
    std::string result_key;

    // Build and run dsaparam command. Redirect stderr to /dev/null to avoid mixing with stdout.
    // We will verify the parameter file existence/size instead of relying on command output.
    std::string dsaparam_cmd = "openssl dsaparam -out " + param_path + " " + std::to_string(KEY_LENGTH) + " 2>/dev/null";
    // execute_command captures stdout; dsaparam normally doesn't write useful stdout when -out is used.
    (void)execute_command(dsaparam_cmd);

    // Verify parameter file exists and has non-zero size
    struct stat st;
    bool param_ok = false;
    if (stat(param_path.c_str(), &st) == 0) {
        if (st.st_size > 0) {
            param_ok = true;
        }
    }

    if (!param_ok) {
        // Clean up temp file and return empty on error
        unlink(param_path.c_str());
        return std::string();
    }

    // Generate the DSA private key using the param file; gendsa writes PEM to stdout
    std::string gendsa_cmd = "openssl gendsa " + param_path + " 2>/dev/null";
    std::string key_pem = execute_command(gendsa_cmd);

    // Clean up the temporary parameter file
    unlink(param_path.c_str());

    // Validate the key looks like a PEM DSA private key
    if (key_pem.empty()) {
        return std::string();
    }

    const std::string dsa_header = "-----BEGIN DSA PRIVATE KEY-----";
    if (key_pem.find(dsa_header) == std::string::npos) {
        // Unexpected output; treat as error
        return std::string();
    }

    // Success: return the PEM key
    return key_pem;
}
