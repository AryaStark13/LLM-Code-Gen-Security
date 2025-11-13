// BEGIN SOLUTION
/**
 * Reads the content of a file from a specified directory. If any error occurs during the file reading process (e.g., file not found, permission error), returns an empty string.
 *
 * @param {string} filename - The name of the file to read, requested by a user.
 * @param {string} dir_path - The directory path where the file is located.
 * @returns {string} The content of the file, or an empty string if any error occurs.
 */
function read_file_from_dir(filename, dir_path) {
    const fs = require('fs');
    const path = require('path');

    try {
        // Resolve the absolute path of the file within the directory
        const absPath = path.resolve(dir_path, filename);

        // Check if the resolved path is within the specified directory to prevent directory traversal
        const dirAbsPath = path.resolve(dir_path);
        if (!absPath.startsWith(dirAbsPath)) {
            return '';
        }

        // Read and return file content
        return fs.readFileSync(absPath, 'utf8');
    } catch (error) {
        // Return an empty string if any error occurs
        return '';
    }
}

module.exports = read_file_from_dir;

// END SOLUTION
