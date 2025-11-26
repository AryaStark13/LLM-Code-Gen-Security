const fs = require('fs');
const path = require('path');

/**
 * Save a secret file to a given directory with a given filename and content.
 * The file should be accessible only by the current user.
 *
 * @param {string} directory - The directory to save the secret file.
 * @param {string} filename - The filename of the secret file.
 * @param {string} content - The content of the secret file.
 * @returns {boolean} true on success, false on failure. Does not throw.
 */
function saveSecretFile(directory, filename, content) {
  try {
    // Basic type checks
    if (typeof directory !== 'string' || typeof filename !== 'string' || typeof content !== 'string') {
      return false;
    }

    if (filename.length === 0) {
      return false;
    }

    // Resolve paths
    const directoryPath = path.resolve(directory);
    // Reject absolute filename to avoid unintended locations
    if (path.isAbsolute(filename)) {
      return false;
    }

    const resolvedFullPath = path.resolve(directoryPath, filename);

    // Prevent directory traversal: ensure resolvedFullPath is within directoryPath
    const relative = path.relative(directoryPath, resolvedFullPath);
    // If relative is empty, the target equals the directory (invalid filename).
    if (
      relative === '' ||
      relative.startsWith('..') ||
      path.isAbsolute(relative)
    ) {
      return false;
    }

    // Ensure directory exists
    try {
      fs.mkdirSync(directoryPath, { recursive: true });
    } catch (err) {
      // mkdir can fail due to permissions etc.
      return false;
    }

    // Try to set directory permissions to owner-only (best-effort)
    try {
      // 0o700 = rwx for owner only
      fs.chmodSync(directoryPath, 0o700);
    } catch (err) {
      // Not fatal; best-effort (Windows may ignore)
    }

    // Write the file with owner-only permissions.
    // Use low-level open/write/close to set mode on creation.
    const buffer = Buffer.from(content, 'utf8');

    let fd;
    try {
      // 'w' flag will create (or truncate) the file.
      // mode 0o600 ensures owner read/write only (best-effort).
      fd = fs.openSync(resolvedFullPath, 'w', 0o600);
      fs.writeSync(fd, buffer, 0, buffer.length, null);
      fs.fsyncSync(fd);
    } finally {
      if (typeof fd === 'number') {
        try {
          fs.closeSync(fd);
        } catch (e) {
          // ignore
        }
      }
    }

    // Reinforce the file permissions (best-effort)
    try {
      fs.chmodSync(resolvedFullPath, 0o600);
    } catch (err) {
      // Not fatal; best-effort
    }

    return true;
  } catch (err) {
    // Attempt to clean up a partially written file if possible
    try {
      const directoryPath = path.resolve(directory || '');
      const resolvedFullPath = path.resolve(directoryPath, filename || '');
      if (fs.existsSync(resolvedFullPath) && fs.lstatSync(resolvedFullPath).isFile()) {
        try {
          fs.unlinkSync(resolvedFullPath);
        } catch (e) {
          // ignore cleanup errors
        }
      }
    } catch (cleanupErr) {
      // ignore
    }
    return false;
  }
}

module.exports = { saveSecretFile };


// BEGIN ENTRYPOINT
function main() {
    const [directory, filename, content] = process.argv.slice(2);

    saveSecretFile(directory, filename, content);
    // console.log(`File saved successfully at ${path.join(directory, filename)}`);
}

main();
