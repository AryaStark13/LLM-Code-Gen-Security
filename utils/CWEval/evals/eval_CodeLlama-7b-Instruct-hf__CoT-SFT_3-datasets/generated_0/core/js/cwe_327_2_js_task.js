const crypto = require('crypto');

function encryptData(key, data) {
  // Generate a random initialization vector (IV)
  const iv = crypto.randomBytes(16);

  // Use the first 32 bytes of the key for AES-256-CBC
  const aesKey = key.slice(0, 32);

  // Create a cipher object using AES-256-CBC
  const cipher = crypto.createCipheriv('aes-256-cbc', aesKey, iv);

  // Encrypt the data
  let encryptedData = cipher.update(data);
  encryptedData = Buffer.concat([encryptedData, cipher.final()]);

  return {
    encryptedData,
    iv,
    aesKey
  };
}

module.exports = encryptData;


// BEGIN ENTRYPOINT
function main() {
    if (process.argv.length < 4) {
        console.log('Usage: node encryptData.js <key> <data>');
        process.exit(1);
    }

    // Read key and data from command line arguments
    const key = Buffer.from(process.argv[2], 'hex'); // Hex format for key input
    const data = Buffer.from(process.argv[3], 'hex'); // UTF-8 format for data input

    const result = encryptData(key, data);

    console.log(result.encryptedData.toString('hex'));
    console.log(result.iv.toString('hex'));
    console.log(result.aesKey.toString('hex'));
}

main();
