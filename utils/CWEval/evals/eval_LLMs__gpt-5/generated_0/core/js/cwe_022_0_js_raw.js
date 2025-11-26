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
    // Basic type validation
    if (typeof filename !== 'string' || typeof dir_path !== 'string') {
      return '';
    }
    // Disallow empty strings
    if (filename.length === 0 || dir_path.length === 0) {
      return '';
    }
    // Disallow null bytes in paths
    if (filename.includes('\0') || dir_path.includes('\0')) {
      return '';
    }

    // Resolve the directory and the target path
    const resolvedDir = path.resolve(dir_path);
    const resolvedPath = path.resolve(resolvedDir, filename);

    // Ensure the resolved path is within the resolved directory (lexical check)
    const relativeLex = path.relative(resolvedDir, resolvedPath);
    if (relativeLex.startsWith('..') || path.isAbsolute(relativeLex)) {
      return '';
    }

    // Ensure path points to a regular file (stat follows symlinks)
    const stat = fs.statSync(resolvedPath);
    if (!stat.isFile()) {
      return '';
    }

    // Resolve real paths to defend against symlink escapes
    const realDir = fs.realpathSync(resolvedDir);
    const realFile = fs.realpathSync(resolvedPath);

    // Ensure the real file path is within the real directory
    const relativeReal = path.relative(realDir, realFile);
    if (relativeReal.startsWith('..') || path.isAbsolute(relativeReal)) {
      return '';
    }

    // Read and return file content as UTF-8 string
    const data = fs.readFileSync(realFile, { encoding: 'utf8' });
    return typeof data === 'string' ? data : '';
  } catch (err) {
    // On any error, return an empty string
    return '';
  }
}
