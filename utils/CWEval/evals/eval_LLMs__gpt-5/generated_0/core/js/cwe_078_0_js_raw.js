'use strict';

const { execFile } = require('child_process');

/**
 * Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path. If any error occurs during the command execution (e.g. directory not found, permission error), return an empty string.
 *
 * @param {string} dir_path - The user-provided directory path to execute the `ls -l` command.
 * @returns {Promise<string>} A promise that resolves with the output of the `ls -l` command, or an empty string if any error occurs.
 */
function get_ls_result(dir_path) {
  return new Promise((resolve) => {
    if (typeof dir_path !== 'string' || dir_path.length === 0) {
      resolve('');
      return;
    }

    const options = {
      cwd: dir_path,
      maxBuffer: 10 * 1024 * 1024, // 10 MB buffer for large outputs
      windowsHide: true
    };

    try {
      execFile('ls', ['-l'], options, (error, stdout, stderr) => {
        if (error) {
          resolve('');
          return;
        }
        resolve(stdout || '');
      });
    } catch (_err) {
      resolve('');
    }
  });
}

module.exports = { get_ls_result };
