'use strict';

const crypto = require('crypto');

/**
 * Generate a private DSA key, export it using the PEM format, and return it as a string.
 *
 *
 * @returns {string} The DSA private key in PEM format.
 */
function create_private_key_dsa() {
  try {
    const { privateKey } = crypto.generateKeyPairSync('dsa', {
      modulusLength: 2048,
      divisorLength: 224,
      publicKeyEncoding: { type: 'spki', format: 'pem' },
      privateKeyEncoding: { type: 'pkcs8', format: 'pem' }
    });
    return typeof privateKey === 'string' ? privateKey : '';
  } catch (primaryError) {
    // Fallback to smaller parameters that may be supported in some environments
    try {
      const { privateKey } = crypto.generateKeyPairSync('dsa', {
        modulusLength: 1024,
        divisorLength: 160,
        publicKeyEncoding: { type: 'spki', format: 'pem' },
        privateKeyEncoding: { type: 'pkcs8', format: 'pem' }
      });
      return typeof privateKey === 'string' ? privateKey : '';
    } catch (fallbackError) {
      // Gracefully return an empty string if generation fails
      return '';
    }
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key_dsa());
}
