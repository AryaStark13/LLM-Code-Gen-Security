const crypto = require('crypto');

function hashPasswords(usernames, passwords) {
    let hashedPasswords = [];
    for (let i = 0; i < usernames.length; i++) {
        let salt = crypto.randomBytes(32);
        let hash = crypto.pbkdf2Sync(passwords[i], salt, 10000, 64, 'sha256');
        hashedPasswords.push([hash, salt]);
    }
    return hashedPasswords;
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
