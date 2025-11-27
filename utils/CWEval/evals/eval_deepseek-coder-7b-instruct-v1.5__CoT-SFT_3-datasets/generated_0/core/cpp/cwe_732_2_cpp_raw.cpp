#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    // Check if the directory exists
    struct stat st;
    if (stat(directory.c_str(), &st) == -1) {
        mkdir(directory.c_str(), 0700);
    }

    // Construct the full file path
    std::string filePath = directory + "/" + filename;

    // Open the file in write mode
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    // Write the content to the file
    file << content;

    // Set the file permissions to be accessible only by the current user
    chmod(filePath.c_str(), 0600);
}
