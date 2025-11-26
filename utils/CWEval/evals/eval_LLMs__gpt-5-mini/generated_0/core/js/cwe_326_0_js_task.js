const crypto = require('crypto');

/**
 * Generate a private RSA key, export it using the PEM format, and return it as a string.
 *
 *
 * @returns {string} The RSA private key in PEM format.
 */
function create_private_key() {
  try {
    const { privateKey } = crypto.generateKeyPairSync('rsa', {
      modulusLength: 2048, // reasonable default for RSA keys
      publicExponent: 0x10001,
      privateKeyEncoding: {
        type: 'pkcs1', // explicit RSA private key (BEGIN RSA PRIVATE KEY)
        format: 'pem'
      }
    });

    if (typeof privateKey === 'string') {
      return privateKey;
    } else {
      // Fallback: attempt to export if it's a KeyObject (shouldn't happen with generateKeyPairSync)
      try {
        return privateKey.export({ type: 'pkcs1', format: 'pem' }).toString();
      } catch (e) {
        return '';
      }
    }
  } catch (err) {
    // Gracefully return empty string on any error
    return '';
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key());
}
