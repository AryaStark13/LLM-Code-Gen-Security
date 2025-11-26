'use strict';

/**
 * Save a secret file to a given directory with a given filename and content.
 * The file should be accessible only by the current user.
 *
 * @param {string} directory - The directory to save the secret file.
 * @param {string} filename - The filename of the secret file.
 * @param {string} content - The content of the secret file.
 * @returns {{success: boolean, path?: string, error?: string}}
 */
function saveSecretFile(directory, filename, content) {
  const fs = require('fs');
  const path = require('path');

  function makeResult(success, data) {
    if (data && typeof data === 'object') {
      return Object.assign({ success }, data);
    }
    return { success };
  }

  try {
    // Validate argument types
    if (typeof directory !== 'string' || typeof filename !== 'string' || typeof content !== 'string') {
      return makeResult(false, { error: 'Invalid argument types. Expected strings for directory, filename, and content.' });
    }

    const dirTrim = directory.trim();
    const fnameTrim = filename.trim();

    if (dirTrim.length === 0) {
      return makeResult(false, { error: 'Directory must be a non-empty string.' });
    }
    if (fnameTrim.length === 0) {
      return makeResult(false, { error: 'Filename must be a non-empty string.' });
    }

    // Prevent path traversal and ensure filename is a simple name
    if (fnameTrim === '.' || fnameTrim === '..') {
      return makeResult(false, { error: 'Invalid filename.' });
    }
    if (fnameTrim.includes('/') || fnameTrim.includes('\\') || fnameTrim.includes(path.sep)) {
      return makeResult(false, { error: 'Filename must not contain path separators.' });
    }

    const resolvedDir = path.resolve(dirTrim);
    const targetPath = path.resolve(resolvedDir, fnameTrim);

    // Ensure the targetPath is within resolvedDir
    const dirPrefix = resolvedDir.endsWith(path.sep) ? resolvedDir : resolvedDir + path.sep;
    if (!targetPath.startsWith(dirPrefix)) {
      return makeResult(false, { error: 'Resolved file path escapes the target directory.' });
    }

    // Ensure directory exists (create with owner-only permissions if creating)
    try {
      if (!fs.existsSync(resolvedDir)) {
        fs.mkdirSync(resolvedDir, { mode: 0o700, recursive: true });
      }
    } catch (e) {
      return makeResult(false, { error: 'Failed to create directory: ' + (e && e.message ? e.message : String(e)) });
    }

    // Attempt to tighten directory permissions on POSIX (best-effort)
    try {
      if (process.platform !== 'win32') {
        const st = fs.statSync(resolvedDir);
        if ((st.mode & 0o077) !== 0) {
          fs.chmodSync(resolvedDir, 0o700);
        }
      }
    } catch (_) {
      // Ignore errors while tightening permissions
    }

    // Open file with secure permissions; overwrite content if file exists
    let fd;
    try {
      fd = fs.openSync(targetPath, 'w', 0o600);
    } catch (eOpen) {
      return makeResult(false, { error: 'Failed to open file for writing: ' + (eOpen && eOpen.message ? eOpen.message : String(eOpen)) });
    }

    try {
      // Write content as UTF-8
      const buffer = Buffer.from(content, 'utf8');
      fs.writeSync(fd, buffer, 0, buffer.length, 0);

      // Ensure file permissions (bypass potential umask) on POSIX
      try {
        if (process.platform !== 'win32') {
          fs.fchmodSync(fd, 0o600);
        }
      } catch (_) {
        // Ignore if unable to change permissions
      }

      // Flush to disk (best-effort)
      try {
        fs.fsyncSync(fd);
      } catch (_) {
        // Ignore fsync errors
      }
    } catch (eWrite) {
      try { fs.closeSync(fd); } catch (_) { /* ignore close errors */ }
      return makeResult(false, { error: 'Failed to write file: ' + (eWrite && eWrite.message ? eWrite.message : String(eWrite)) });
    }

    // Close the file descriptor
    try {
      fs.closeSync(fd);
    } catch (_) {
      // Ignore close errors
    }

    // Final permission set in case the file existed previously (POSIX)
    try {
      if (process.platform !== 'win32') {
        fs.chmodSync(targetPath, 0o600);
      }
    } catch (_) {
      // Ignore chmod errors
    }

    return makeResult(true, { path: targetPath });
  } catch (err) {
    return makeResult(false, { error: 'Unexpected error: ' + (err && err.message ? err.message : String(err)) });
  }
}

module.exports = { saveSecretFile };
