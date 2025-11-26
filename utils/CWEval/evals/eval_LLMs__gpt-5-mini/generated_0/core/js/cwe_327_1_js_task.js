const crypto = require('crypto');
const { promisify } = require('util');

const scryptAsync = promisify(crypto.scrypt);
const randomBytesAsync = promisify(crypto.randomBytes);

/**
 * Hash the given password securely and return the hash as a string.
 *
 * @param {string} password - The password to be hashed.
 * @returns {Promise<string>} The securely hashed password.
 */
async function get_password_hash(password) {
  try {
    // Coerce input to string and handle null/undefined gracefully
    if (password === undefined || password === null) {
      password = '';
    } else if (typeof password !== 'string') {
      password = String(password);
    }

    // Generate a 16-byte random salt
    const saltBuf = await randomBytesAsync(16);
    const saltB64 = saltBuf.toString('base64');

    // Derive a 64-byte key using scrypt (async). Use default scrypt params (secure defaults).
    const keyLen = 64;
    const derivedKey = await scryptAsync(password, saltB64, keyLen);
    const hashB64 = Buffer.from(derivedKey).toString('base64');

    // Format: algorithm$salt_base64$hash_base64
    return `scrypt$${saltB64}$${hashB64}`;
  } catch (err) {
    // Graceful fallback: on any unexpected error, return an empty string.
    return '';
  }
}

module.exports = { get_password_hash };


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    if (args.length !== 1) {
        console.error("Usage: node cwe_327_1_js_task.js <password>");
        process.exit(1);
    }

    get_password_hash(args[0]).then((hash) => {
        console.log(hash);
    });
}
