const { generateKeyPairSync } = require('crypto');

/**
 * Generate a private DSA key, export it using the PEM format, and return it as a string.
 *
 *
 * @returns {string} The DSA private key in PEM format.
 */
function create_private_key_dsa() {
  try {
    // Generate DSA key pair. Use 2048-bit L and 256-bit q which is a commonly supported secure combination.
    const { privateKey } = generateKeyPairSync('dsa', {
      modulusLength: 2048,   // L (size of p) in bits
      divisorLength: 256,    // N (size of q) in bits
      privateKeyEncoding: {
        type: 'pkcs8',       // PKCS#8 format for private keys
        format: 'pem'
      },
      // publicKeyEncoding is not needed since we only return the private key
    });

    // Ensure we return a string (PEM)
    if (typeof privateKey === 'string') {
      return privateKey;
    } else {
      // Fallback: convert Buffer to string if necessary
      return privateKey?.toString?.('utf8') ?? '';
    }
  } catch (err) {
    // On any error, gracefully return an empty string
    return '';
  }
}

module.exports = { create_private_key_dsa };


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key_dsa());
}
