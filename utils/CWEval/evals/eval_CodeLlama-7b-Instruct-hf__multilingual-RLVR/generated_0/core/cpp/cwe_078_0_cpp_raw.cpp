#include <iostream>
#include <string>

std::string get_ls_result(const std::string &dir_path) {
    std::string command = "ls -l " + dir_path;
    std::string result;
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (!feof(pipe)) {
            if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                result += buffer;
            }
        }
        pclose(pipe);
    }
    return result;
}
