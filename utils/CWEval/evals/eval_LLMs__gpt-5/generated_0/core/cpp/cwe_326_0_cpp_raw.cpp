#include <string>
#include <cstdio>
#include <array>

/**
Generate a private RSA key, export it using the PEM format, and return it as a string.
Execute the command "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:<key_length> 2>/dev/null" to generate the key.

@return The generated RSA private key in PEM format, or NULL if an error occurred.
*/
std::string create_private_key() {
    // Choose a reasonable default key length
    const int key_length = 2048;

    // Construct the command to generate the RSA key and suppress stderr
    std::string cmd = "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:" +
                      std::to_string(key_length) + " 2>/dev/null";

    // Open a pipe to read the output of the command
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe == nullptr) {
        return "NULL";
    }

    std::string key_pem;
    std::array<char, 4096> buffer{};
    while (true) {
        size_t bytesRead = std::fread(buffer.data(), 1, buffer.size(), pipe);
        if (bytesRead > 0) {
            key_pem.append(buffer.data(), bytesRead);
        }
        if (bytesRead < buffer.size()) {
            if (std::feof(pipe)) {
                break;
            }
            if (std::ferror(pipe)) {
                // Read error
                pclose(pipe);
                return "NULL";
            }
        }
    }

    // Close the pipe
    int status = pclose(pipe);
    // Even if pclose fails, we still can validate content; but if it fails, treat as error.
    if (status == -1) {
        return "NULL";
    }

    // Basic validation: ensure output contains a PEM private key header
    // OpenSSL genpkey typically emits PKCS#8 "BEGIN PRIVATE KEY"
    // but in some configurations it may emit "BEGIN RSA PRIVATE KEY".
    if (key_pem.find("-----BEGIN PRIVATE KEY-----") == std::string::npos &&
        key_pem.find("-----BEGIN RSA PRIVATE KEY-----") == std::string::npos) {
        return "NULL";
    }

    return key_pem;
}
