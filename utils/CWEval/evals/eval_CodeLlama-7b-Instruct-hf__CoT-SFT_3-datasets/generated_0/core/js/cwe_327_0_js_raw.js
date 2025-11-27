const crypto = require('crypto');

function hash_certificate(cert) {
  const hash = crypto.createHash('sha256');
  hash.update(cert);
  return hash.digest('hex');
}

module.exports = hash_certificate;
