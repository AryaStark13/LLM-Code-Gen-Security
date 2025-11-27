const crypto = require('crypto');

/**
 * Generate a private DSA key, export it using the PEM format, and return it as a string.
 *
 *
 * @returns {string} The DSA private key in PEM format.
 */
function create_private_key_dsa() {
  try {
    const key = crypto.generateKeyPairSync('dsa', {
      modulusLength: 2048,
      publicKeyEncoding: {
        type: 'spki',
        format: 'pem'
      },
      privateKeyEncoding: {
        type: 'pkcs1',
        format: 'pem'
      }
    });
    return key.privateKey;
  } catch (err) {
    console.error('Error generating private key:', err);
    return '';
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key_dsa());
}
