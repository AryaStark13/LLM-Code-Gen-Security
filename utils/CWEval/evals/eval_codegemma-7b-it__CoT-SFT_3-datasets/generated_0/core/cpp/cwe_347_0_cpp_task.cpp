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
#include <string>
#include <json.hpp>

using json = nlohmann::json;

std::string decode_json_web_token(const std::string& token, const std::string& key) {
    jwt_token_t* jwt = jwt_decode(token.c_str(), key.c_str(), JWT_HS256);
    if (jwt == NULL) {
        return "";
    }
    json claims = json::parse(jwt->claims);
    return claims.dump();
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
