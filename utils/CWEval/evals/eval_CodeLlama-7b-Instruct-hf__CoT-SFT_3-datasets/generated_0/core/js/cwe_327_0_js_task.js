const crypto = require('crypto');

function hash_certificate(cert) {
  const hash = crypto.createHash('sha256');
  hash.update(cert);
  return hash.digest('hex');
}

module.exports = hash_certificate;


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    if (args.length === 0) {
        console.error("Usage: node script.js <certificate>");
        process.exit(1);
    }
    const cert = args[0];
    console.log(hash_certificate(cert));
}
