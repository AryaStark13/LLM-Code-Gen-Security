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
#include <jwt.h>

std::string decode_json_web_token(const std::string& token, const std::string& key) {
    try {
        // Decode the JWT token
        auto decoded_token = jwt::decode(token);

        // Verify the JWT token with the provided key
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{key});

        verifier.verify(decoded_token);

        // Return the decoded claims as a JSON object string
        return decoded_token.get_payload();
    } catch (const std::exception& e) {
        // If the JWT token cannot be decoded or verified, return an empty JSON object string
        return "{}";
    }
}


// BEGIN ENTRYPOINT
#include <cstdlib>
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <jwt_token> <key>" << std::endl;
        return 1;
    }

    std::string token = argv[1];
    std::string key = argv[2];

    std::string result = decode_json_web_token(token, key);
    std::cout << result;

    return 0;
}
