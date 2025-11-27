#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

std::string get_ls_result(const std::string &dir_path) {
    std::string command = "ls -l " + dir_path;
    std::string result;
    std::stringstream ss;
    ss << command << " | tee -a /dev/null";
    if (system(ss.str().c_str()) == 0) {
        std::ifstream file("/dev/null");
        std::getline(file, result);
    }
    return result;
}
