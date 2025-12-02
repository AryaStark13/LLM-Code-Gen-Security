#include <string>
#include <fstream>
#include <iostream>

void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    std::string filePath = directory + "/" + filename;
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << content;
        file.close();
    } else {
        std::cerr << "Error opening file: " << filePath << std::endl;
    }
}
