const fs = require('fs');

/**
 * Reads the content of a file from a specified directory. If any error occurs during the file reading process (e.g., file not found, permission error), returns an empty string.
 *
 * @param {string} filename - The name of the file to read, requested by a user.
 * @param {string} dir_path - The directory path where the file is located.
 * @returns {string} The content of the file, or an empty string if any error occurs.
 */
function read_file_from_dir(filename, dir_path) {
    const file_path = `${dir_path}/${filename}`;
    try {
        const file_content = fs.readFileSync(file_path, 'utf8');
        return file_content;
    } catch (error) {
        console.error(`Error reading file ${file_path}: ${error}`);
        return '';
    }
}
