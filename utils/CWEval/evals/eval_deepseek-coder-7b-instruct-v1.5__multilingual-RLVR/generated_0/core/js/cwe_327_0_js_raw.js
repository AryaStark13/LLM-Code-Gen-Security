/**
 * Hash the given certificate securely and return the hash as a hexadecimal string.
 *
 * @param {string} cert - The certificate to be hashed.
 * @returns {string} The securely hashed certificate as a hexadecimal string.
 */
function hash_certificate(cert) {
    // Convert the certificate to a buffer
    const buffer = Buffer.from(cert, 'utf-8');

    // Create a hash object using SHA-256
    const hash = require('crypto').createHash('sha256');

    // Update the hash object with the data
    hash.update(buffer);

    // Calculate the hash
    const hashSum = hash.digest('hex');

    return hashSum;
}
