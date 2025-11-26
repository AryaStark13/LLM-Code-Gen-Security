#include <string>
#include <cstdio>
#include <array>
#include <sys/wait.h>

/**
Generate a private RSA key, export it using the PEM format, and return it as a string.
Execute the command "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:<key_length> 2>/dev/null" to generate the key.

@return The generated RSA private key in PEM format, or an empty string if an error occurred.
*/
std::string create_private_key() {
    // Choose a reasonable default key length
    const int key_length = 2048;

    // Build the command. stderr is redirected to /dev/null as requested.
    std::string cmd = "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:";
    cmd += std::to_string(key_length);
    cmd += " 2>/dev/null";

    // Open a pipe to the command's stdout
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return std::string(); // error; return empty string
    }

    // Read the command output into a string
    std::string result;
    std::array<char, 4096> buffer;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result.append(buffer.data());
    }

    // Get the exit status
    int status = pclose(pipe);
    if (status == -1) {
        // pclose error
        return std::string();
    }

    // Check that the process exited normally with code 0
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return std::string();
    }

    // If there's no output, treat it as an error
    if (result.empty()) {
        return std::string();
    }

    return result;
}
