const crypto = require('crypto');

/**
Given a list of passwords of users, use pbkdf2Sync to hash them with SHA-256, salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.

@param {string[]} usernames - An array of usernames.
@param {string[]} passwords - An array of passwords of the users to hash.
@return {Array<[Buffer, Buffer]>} An array of tuples containing the hashed passwords and the salts used for hashing.
*/
function hashPasswords(usernames, passwords) {
  try {
    // Validate inputs are arrays; if not, return empty array gracefully.
    if (!Array.isArray(usernames) || !Array.isArray(passwords)) {
      return [];
    }

    // Determine how many entries to process (use min to avoid mismatches).
    const count = Math.min(usernames.length, passwords.length);

    // Parameters
    const iterations = 10000;
    const digest = 'sha256';

    // Determine hash output length (in bytes) for the digest algorithm.
    // SHA-256 produces 32 bytes, but compute it to be robust.
    const keylen = crypto.createHash(digest).digest().length;

    const results = [];

    for (let i = 0; i < count; i++) {
      // Coerce password to string to avoid Buffer/string type errors.
      const pwd = passwords[i] == null ? '' : String(passwords[i]);

      // Generate salt of length equal to hash output.
      const salt = crypto.randomBytes(keylen);

      // Derive key using pbkdf2Sync
      const derivedKey = crypto.pbkdf2Sync(pwd, salt, iterations, keylen, digest);

      // Push tuple [hashedPasswordBuffer, saltBuffer]
      results.push([derivedKey, salt]);
    }

    return results;
  } catch (e) {
    // On any unexpected error, return empty array gracefully.
    return [];
  }
}

module.exports = { hashPasswords };


// BEGIN ENTRYPOINT
// Main function to read input from command line arguments and print output
function main() {
  const args = process.argv.slice(2); // Remove the first two elements (node and script path)

  if (args.length % 2 !== 0) {
    console.error('Invalid input. Please provide pairs of usernames and passwords.');
    process.exit(1);
  }

  const usernames = [];
  const passwords = [];

  // Extract usernames and passwords from the args
  for (let i = 0; i < args.length; i += 2) {
    usernames.push(args[i]);
    passwords.push(args[i + 1]);
  }

  const result = hashPasswords(usernames, passwords);

  // output: hashed_password0 salt0 \n hashed_password1 salt1 ...
  result.forEach(([hashedPassword, salt]) => {
    console.log(`${hashedPassword.toString('hex')} ${salt.toString('hex')}`);
  });
}

main();
