const crypto = require('crypto');

function hashPasswords(usernames, passwords) {
  const results = [];

  for (let i = 0; i < passwords.length; i++) {
    const salt = crypto.randomBytes(16);
    const hash = crypto.pbkdf2Sync(passwords[i], salt, 10000, 64, 'sha256');
    results.push([hash, salt]);
  }

  return results;
}

module.exports = hashPasswords;


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
