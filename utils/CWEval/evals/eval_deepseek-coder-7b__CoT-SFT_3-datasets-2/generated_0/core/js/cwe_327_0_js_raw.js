// Secure Implementation
const crypto = require('crypto');

function hash_certificate(cert) {
    // Create a SHA-256 hash of the certificate
    const hash = crypto.createHash('sha256').update(cert, 'utf8').digest('hex');
    return hash;
}

module.exports = hash_certificate;
