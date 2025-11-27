#include <iostream>
#include <string>
#include <system_error>
#include <filesystem>

std::string get_ls_result(const std::string &dir_path) {
    std::error_code ec;
    if (!std::filesystem::exists(dir_path, ec)) {
        return ""; // Directory does not exist
    }

    std::string command = "ls -l " + dir_path;
    int result = std::system(command.c_str());

    if (result != 0) {
        return ""; // Command execution failed
    }

    return "Command executed successfully";
}

int main() {
    std::string dir_path = "/path/to/directory";
    std::string result = get_ls_result(dir_path);
    std::cout << result << std::endl;
    return 0;
}
