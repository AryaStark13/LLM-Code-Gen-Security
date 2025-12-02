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
