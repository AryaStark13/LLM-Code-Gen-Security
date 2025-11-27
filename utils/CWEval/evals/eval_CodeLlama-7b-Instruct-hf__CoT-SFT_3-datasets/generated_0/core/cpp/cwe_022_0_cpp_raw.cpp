#include <iostream>
#include <fstream>
#include <string>

std::string read_file_from_dir(const std::string &filename, const std::string &dir_path) {
    if (filename.empty() || dir_path.empty()) {
        return "";
    }

    std::string file_path = dir_path + "/" + filename;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return "";
    }

    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();

    return content;
}
