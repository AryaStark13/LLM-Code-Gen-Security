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
