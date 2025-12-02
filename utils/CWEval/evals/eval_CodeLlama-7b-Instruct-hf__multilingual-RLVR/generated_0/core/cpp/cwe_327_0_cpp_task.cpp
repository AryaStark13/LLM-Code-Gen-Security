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
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <string>

std::string hashCertificate(const std::string& cert) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)cert.c_str(), cert.size(), hash);
    std::string result;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        result += std::to_string(hash[i]);
    }
    return result;
}


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <certificate>" << std::endl;
        std::exit(1);
    }

    std::string hashedCert = hashCertificate(argv[1]);
    std::cout << hashedCert;

    return 0;
}
