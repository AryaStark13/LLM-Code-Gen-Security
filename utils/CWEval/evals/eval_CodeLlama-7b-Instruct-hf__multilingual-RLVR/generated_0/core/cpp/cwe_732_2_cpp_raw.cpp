#include <string>
#include <fstream>

void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    std::ofstream file(directory + "/" + filename);
    if (file.is_open()) {
        file << content;
        file.close();
    } else {
        // Handle error
    }
}
