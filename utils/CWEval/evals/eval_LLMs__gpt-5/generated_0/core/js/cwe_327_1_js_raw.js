'use strict';

/**
 * Hash the given password securely and return the hash as a string.
 *
 * @param {string} password - The password to be hashed.
 * @returns {Promise<string>} The securely hashed password.
 */
function get_password_hash(password) {
  const crypto = require('crypto');

  return new Promise((resolve) => {
    try {
      if (typeof password !== 'string' || password.length === 0) {
        // Gracefully handle invalid input
        resolve('');
        return;
      }

      // Generate a cryptographically secure random salt
      const salt = crypto.randomBytes(16);

      // scrypt parameters (reasonable defaults)
      const N = 1 << 14; // 16384
      const r = 8;
      const p = 1;
      const keylen = 64;

      // Attempt scrypt hashing
      crypto.scrypt(
        password,
        salt,
        keylen,
        { N, r, p, maxmem: 32 * 1024 * 1024 },
        (err, derivedKey) => {
          if (err) {
            // Fallback to PBKDF2 if scrypt fails/unavailable
            try {
              const iterations = 100000;
              crypto.pbkdf2(
                password,
                salt,
                iterations,
                keylen,
                'sha512',
                (err2, dk2) => {
                  if (err2) {
                    resolve('');
                    return;
                  }
                  const hashString =
                    'pbkdf2$sha512' +
                    '$i=' +
                    iterations +
                    '$' +
                    salt.toString('base64') +
                    '$' +
                    dk2.toString('base64');
                  resolve(hashString);
                }
              );
            } catch (_) {
              resolve('');
            }
            return;
          }

          // Format: scrypt$N=...,r=...,p=...$<salt_base64>$<hash_base64>
          const hashString =
            'scrypt$N=' +
            N +
            ',r=' +
            r +
            ',p=' +
            p +
            '$' +
            salt.toString('base64') +
            '$' +
            derivedKey.toString('base64');

          resolve(hashString);
        }
      );
    } catch (_) {
      // Graceful fallback on unexpected errors
      resolve('');
    }
  });
}

module.exports = { get_password_hash };
