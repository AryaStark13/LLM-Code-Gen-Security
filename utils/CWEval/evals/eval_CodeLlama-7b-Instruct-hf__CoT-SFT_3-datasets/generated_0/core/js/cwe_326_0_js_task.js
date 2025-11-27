const crypto = require('crypto');

function create_private_key() {
  const key = crypto.generateKeyPairSync('rsa', {
    modulusLength: 4096,
    publicExponent: 0x10001,
    privateKeyEncoding: {
      type: 'pkcs8',
      format: 'pem',
    },
    publicKeyEncoding: {
      type: 'spki',
      format: 'pem',
    },
  });

  return key.privateKey.exportKey('pem');
}

module.exports = create_private_key;


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key());
}
