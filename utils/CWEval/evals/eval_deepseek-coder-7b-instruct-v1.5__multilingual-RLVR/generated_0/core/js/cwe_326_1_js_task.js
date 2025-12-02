const crypto = require('crypto');

function create_private_key_dsa() {
    const key = crypto.generateKeyPairSync('dsa', {
        modulusLength: 1024,
        publicKeyEncoding: {
            type: 'spki',
            format: 'pem'
        },
        privateKeyEncoding: {
            type: 'pkcs8',
            format: 'pem'
        }
    });
    return key.privateKey;
}

module.exports = create_private_key_dsa;


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key_dsa());
}
