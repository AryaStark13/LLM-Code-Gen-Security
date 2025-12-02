const crypto = require('crypto');
const pem = require('pem');

function create_private_key() {
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

module.exports = create_private_key;


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key());
}
