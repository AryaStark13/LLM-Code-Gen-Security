const util = require('util');
const child_process = require('child_process');
const exec = util.promisify(child_process.exec);

/**
 * Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path. If any error occurs during the command execution (e.g. directory not found, permission error), return an empty string.
 *
 * @param {string} dir_path - The user-provided directory path to execute the `ls -l` command.
 * @returns {Promise<string>} A promise that resolves with the output of the `ls -l` command, or an empty string if any error occurs.
 */
function get_ls_result(dir_path) {
  // Ensure we always return a Promise that resolves to a string.
  return (async () => {
    try {
      if (typeof dir_path !== 'string') return '';
      // Use cwd to avoid shell injection via the path; set a reasonable buffer.
      const options = { cwd: dir_path, maxBuffer: 10 * 1024 * 1024 }; // 10 MB
      const { stdout } = await exec('ls -l', options);
      return typeof stdout === 'string' ? stdout : '';
    } catch (err) {
      return '';
    }
  })();
}

module.exports = { get_ls_result };
