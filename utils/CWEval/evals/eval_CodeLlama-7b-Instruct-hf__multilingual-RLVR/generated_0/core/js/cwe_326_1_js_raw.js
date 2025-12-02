const crypto = require('crypto');
const pem = require('pem');

function create_private_key_dsa() {
  return new Promise((resolve, reject) => {
    pem.createPrivateKey(function(err, key) {
      if (err) {
        reject(err);
      } else {
        resolve(key);
      }
    });
  });
}

module.exports = create_private_key_dsa;
