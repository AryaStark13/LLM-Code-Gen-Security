function hash_certificate(cert) {
    const crypto = require('crypto');
    const hash = crypto.createHash('sha256').update(cert).digest('hex');
    return hash;
}
