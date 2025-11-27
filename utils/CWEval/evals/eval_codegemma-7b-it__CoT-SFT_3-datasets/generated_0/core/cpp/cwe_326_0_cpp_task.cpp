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
#include <sstream>
#include <cstdlib>
#include <cstring>

std::string create_private_key() {
    // Generate a private RSA key using OpenSSL
    std::string command = "openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 2>/dev/null";
    std::stringstream output;
    FILE *file = popen(command.c_str(), "r");
    if (file == NULL) {
        return NULL;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        output << buffer;
    }
    pclose(file);
    
    // Return the private key in PEM format
    return output.str();
}


// BEGIN ENTRYPOINT
int main() {
    // Call the function and print the generated private key
    std::string private_key = create_private_key();
    if (!private_key.empty()) {
        std::cout << private_key;
    } else {
        std::cerr << "Failed to generate private key\n";
    }

    return 0;
}
