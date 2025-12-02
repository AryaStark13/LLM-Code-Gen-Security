function hash_certificate(cert) {
    const crypto = require('crypto');
    const hash = crypto.createHash('sha256');
    hash.update(cert);
    return hash.digest('hex');
}
