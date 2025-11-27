const crypto = require('crypto');

function create_private_key_dsa() {
  const hash = crypto.createHash('sha256');
  const key = crypto.createPrivateKey({
    key: hash,
    format: 'pem',
    type: 'pkcs8',
  });

  return key;
}

module.exports = create_private_key_dsa;


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key_dsa());
}
