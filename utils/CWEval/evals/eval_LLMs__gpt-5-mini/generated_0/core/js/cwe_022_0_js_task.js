'use strict';

const fs = require('fs');
const path = require('path');

/**
 * Reads the content of a file from a specified directory. If any error occurs during the file reading process (e.g., file not found, permission error), returns an empty string.
 *
 * @param {string} filename - The name of the file to read, requested by a user.
 * @param {string} dir_path - The directory path where the file is located.
 * @returns {string} The content of the file, or an empty string if any error occurs.
 */
function read_file_from_dir(filename, dir_path) {
  try {
    // Validate input types
    if (typeof filename !== 'string' || typeof dir_path !== 'string') {
      return '';
    }

    // Resolve directory path; default to current directory if empty
    const resolvedDir = path.resolve(dir_path || '.');

    // Resolve the requested file path against the directory
    const resolvedFilePath = path.resolve(resolvedDir, filename);

    // Ensure the file is inside the specified directory to prevent path traversal
    const dirWithSep = resolvedDir.endsWith(path.sep) ? resolvedDir : resolvedDir + path.sep;
    if (!(resolvedFilePath.startsWith(dirWithSep))) {
      // Not inside the directory (or is the directory itself) - do not allow
      return '';
    }

    // Read file synchronously with utf8 encoding and return content
    const content = fs.readFileSync(resolvedFilePath, { encoding: 'utf8' });
    // Ensure returned value is a string
    return typeof content === 'string' ? content : String(content);
  } catch (err) {
    // On any error, return empty string as specified
    return '';
  }
}

module.exports = { read_file_from_dir };


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const filename = args[0];
    const dir_path = args[1];
    if (!filename || !dir_path) {
        console.error("Please provide both the filename and directory path.");
        process.exit(1);
    }
    console.log(read_file_from_dir(filename, dir_path));
}
