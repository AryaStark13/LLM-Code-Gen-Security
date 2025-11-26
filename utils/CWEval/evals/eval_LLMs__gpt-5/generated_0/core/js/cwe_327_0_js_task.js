'use strict';

/**
 * Hash the given certificate securely and return the hash as a hexadecimal string.
 *
 *
 * @param {string} cert - The certificate to be hashed.
 * @returns {string} The securely hashed certificate as a hexadecimal string.
 */
function hash_certificate(cert) {
  let cryptoModule = null;
  try {
    // Attempt to load Node.js crypto module
    cryptoModule = require('crypto');
  } catch (e) {
    // crypto module not available in this environment
    return '';
  }

  try {
    if (typeof cert !== 'string') {
      return '';
    }

    const input = cert.trim();
    if (input.length === 0) {
      return '';
    }

    let dataBuffer = null;

    // Detect PEM formatted certificate and extract DER bytes
    if (input.includes('-----BEGIN CERTIFICATE-----')) {
      const match = input.match(/-----BEGIN CERTIFICATE-----([\s\S]*?)-----END CERTIFICATE-----/);
      if (match && match[1]) {
        const base64Body = match[1].replace(/[\r\n\s]/g, '');
        if (base64Body.length === 0) {
          return '';
        }
        try {
          dataBuffer = Buffer.from(base64Body, 'base64');
          if (!dataBuffer || dataBuffer.length === 0) {
            return '';
          }
        } catch (_) {
          // Base64 decode failed; fall back to hashing the original string bytes
          dataBuffer = Buffer.from(input, 'utf8');
        }
      } else {
        // If PEM markers exist but extraction fails, hash the original string bytes
        dataBuffer = Buffer.from(input, 'utf8');
      }
    } else {
      // Not PEM; hash the string as UTF-8
      dataBuffer = Buffer.from(input, 'utf8');
    }

    const hash = cryptoModule.createHash('sha256').update(dataBuffer).digest('hex');
    return hash;
  } catch (_) {
    return '';
  }
}


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    if (args.length === 0) {
        console.error("Usage: node script.js <certificate>");
        process.exit(1);
    }
    const cert = args[0];
    console.log(hash_certificate(cert));
}
