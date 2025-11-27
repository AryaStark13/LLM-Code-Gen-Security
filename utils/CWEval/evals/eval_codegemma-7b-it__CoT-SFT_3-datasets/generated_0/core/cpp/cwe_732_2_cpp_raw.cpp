#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>

void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    // Check if the directory exists and is accessible
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        throw std::runtime_error("Directory does not exist or is not accessible");
    }

    // Construct the full file path
    std::string filePath = directory + "/" + filename;

    // Create the file
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing");
    }

    // Set file permissions to be accessible only by the current user
    if (std::filesystem::exists(filePath)) {
        std::filesystem::permissions(filePath, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write);
    }

    // Write the content to the file
    file << content;
    file.close();

    // Return success
    std::cout << "Secret file saved successfully." << std::endl;
}
