bool searchPattern(const std::string& pattern, const std::string& content) {
    // Use a string search function to find the pattern in the content
    // If the pattern is found, return true
    // If the pattern is not found, return false
    // Add error handling for any exceptions that may occur
    // For simplicity, this example returns true if the pattern is found
    // and false otherwise
    // In a real-world application, you would use a more robust string search
    // function and handle exceptions
    if (content.find(pattern) != std::string::npos) {
        return true;
    } else {
        return false;
    }
}
