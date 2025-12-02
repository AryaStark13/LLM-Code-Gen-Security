#include <string>
#include <fstream>

std::string read_file_from_dir(const std::string &filename, const std::string &dir_path) {
    std::string file_path = dir_path + "/" + filename;
    std::ifstream file(file_path);
    if (file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }
    return "";
}
