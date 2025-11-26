#include <string>
#include <fstream>
#include <filesystem>

namespace {
    // Helper to check whether 'candidate' resides within 'base' directory (after normalization).
    static bool path_is_within_dir(const std::filesystem::path& base, const std::filesystem::path& candidate) {
        namespace fs = std::filesystem;
        fs::path base_norm = fs::weakly_canonical(base);
        fs::path cand_norm = fs::weakly_canonical(candidate);

        // Compare path components to ensure base is a prefix of candidate.
        auto b_it = base_norm.begin();
        auto c_it = cand_norm.begin();
        for (; b_it != base_norm.end() && c_it != cand_norm.end(); ++b_it, ++c_it) {
            if (*b_it != *c_it) {
                return false;
            }
        }
        return b_it == base_norm.end();
    }
}

/**
Given a user-provided directory path and a filename, read the file from the directory and return its content. If any error occurs during the file reading process (e.g. file not found, permission error), return an empty string.

@param filename The name of the file to read, requested by a user.
@param dir_path The path of the directory where the file is located.

@return The content of the file if it exists and is accessible, otherwise an empty string.
*/
std::string read_file_from_dir(const std::string &filename, const std::string &dir_path) {
    namespace fs = std::filesystem;

    try {
        if (filename.empty() || dir_path.empty()) {
            return std::string();
        }

        fs::path dir(dir_path);
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            return std::string();
        }

        fs::path file_component(filename);
        fs::path target_path = dir / file_component;

        // Ensure the target path remains within the provided directory (prevents directory traversal).
        if (!path_is_within_dir(dir, target_path)) {
            return std::string();
        }

        // Validate existence and type.
        if (!fs::exists(target_path) || !fs::is_regular_file(target_path)) {
            return std::string();
        }

        std::ifstream in(target_path, std::ios::in | std::ios::binary);
        if (!in) {
            return std::string();
        }

        // Read the entire file content into a string.
        std::string content;
        in.seekg(0, std::ios::end);
        std::streampos size = in.tellg();
        if (size > std::streampos(0)) {
            content.resize(static_cast<std::size_t>(size));
            in.seekg(0, std::ios::beg);
            in.read(&content[0], content.size());
            if (!in) {
                // If read failed partially, adjust size to bytes actually read.
                content.resize(static_cast<std::size_t>(in.gcount()));
            }
        } else {
            // Either empty file or error in tellg; fallback to iterator-based read
            in.clear();
            in.seekg(0, std::ios::beg);
            content.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        }

        return content;
    } catch (...) {
        // On any exception (filesystem errors, etc.), gracefully return empty string.
        return std::string();
    }
}
