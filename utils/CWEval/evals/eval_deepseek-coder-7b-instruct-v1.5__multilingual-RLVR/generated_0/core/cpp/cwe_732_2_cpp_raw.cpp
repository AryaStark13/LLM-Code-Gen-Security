#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    std::string filePath = directory + "/" + filename;
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }
    file << content;
    file.close();

    // Set file permissions to make it accessible only to the current user
    chmod(filePath.c_str(), S_IRUSR | S_IWUSR);
}
