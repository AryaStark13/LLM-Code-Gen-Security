// Generate a DSA key pair with a secure key size (2048 bits)
const { generateKeyPairSync } = require('crypto');
const { privateKey } = generateKeyPairSync('dsa', {
    modulusLength: 2048, // Key size in bits
});
return privateKey.export({ type: 'pkcs8', format: 'pem' });
