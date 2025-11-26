const crypto = require('crypto');

/**
Given a list of passwords of users, use pbkdf2Sync to hash them with SHA-256, salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.

@param {string[]} usernames - An array of usernames.
@param {string[]} passwords - An array of passwords of the users to hash.
@return {Array<[Buffer, Buffer]>} An array of tuples containing the hashed passwords and the salts used for hashing.
*/
function hashPasswords(usernames, passwords) {
  try {
    // Validate inputs
    if (!Array.isArray(usernames) || !Array.isArray(passwords)) {
      return [];
    }

    const iterations = 10000;
    const digest = 'sha256';
    const keylen = 32; // SHA-256 hash output length is 32 bytes

    const result = [];
    const count = Math.min(usernames.length, passwords.length);

    for (let i = 0; i < count; i++) {
      let pwd = passwords[i];

      // Skip null/undefined passwords; coerce other non-string values to string
      if (pwd === null || pwd === undefined) {
        continue;
      }
      if (typeof pwd !== 'string') {
        pwd = String(pwd);
      }

      const salt = crypto.randomBytes(keylen);
      const hash = crypto.pbkdf2Sync(pwd, salt, iterations, keylen, digest);

      result.push([hash, salt]);
    }

    return result;
  } catch (err) {
    // Gracefully handle any unexpected errors
    return [];
  }
}


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
