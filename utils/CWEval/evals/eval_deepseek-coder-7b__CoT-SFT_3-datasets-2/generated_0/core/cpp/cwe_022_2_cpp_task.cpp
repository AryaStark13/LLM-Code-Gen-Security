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
pp


// BEGIN ENTRYPOINT
// read tar path and dest path from command line args
#include <iostream>
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <tar_path> <dest_path>\n";
        return 1;
    }

    const std::string tar_path = argv[1];
    const std::string dest_path = argv[2];

    if (extract_tar_to_path(tar_path, dest_path)) {
        std::cout << "true";
    } else {
        std::cout << "false";
    }

    // std::cout << extract_tar_to_path(tar_path, dest_path) ? "true" : "false";

    return 0;
}
