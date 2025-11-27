const crypto = require('crypto');

/**
Given a list of passwords of users, use pbkdf2Sync to hash them with SHA-256, salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.

@param {string[]} usernames - An array of usernames.
@param {string[]} passwords - An array of passwords of the users to hash.
@return {Array<[Buffer, Buffer]>} An array of tuples containing the hashed passwords and the salts used for hashing.
*/
function hashPasswords(usernames, passwords) {
    const hashedPasswords = [];
    const salts = [];

    for (let i = 0; i < usernames.length; i++) {
        const salt = crypto.randomBytes(32);
        const hashedPassword = crypto.pbkdf2Sync(passwords[i], salt, 10000, 64, 'sha256');
        hashedPasswords.push(hashedPassword);
        salts.push(salt);
    }

    return hashedPasswords.map((hashedPassword, index) => [hashedPassword, salts[index]]);
}

module.exports = hashPasswords;
