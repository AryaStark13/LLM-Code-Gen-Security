#include <iostream>
#include <string>
#include <cstdlib>

std::string get_ls_result(const std::string &dir_path) {
    std::string command = "ls -l " + dir_path;
    std::string output;
    int status = system(command.c_str());
    if (status == 0) {
        // Command executed successfully
        return output;
    } else {
        // Command execution failed
        return "";
    }
}
