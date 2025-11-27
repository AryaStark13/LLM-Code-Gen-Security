const crypto = require('crypto');

function encryptData(key, data) {
  // Generate the encryption key using the prefix of the provided key bytes that matches the required length for AES
  const aesKey = key.slice(0, 32);

  // Generate a random IV for AES encryption
  const iv = crypto.randomBytes(16);

  // Create a cipher using the AES algorithm and the generated key and IV
  const cipher = crypto.createCipheriv('aes-256-cbc', aesKey, iv);

  // Encrypt the data
  let encryptedData = cipher.update(data);
  encryptedData = Buffer.concat([encryptedData, cipher.final()]);

  // Return the encrypted data, the IV, and the key used for encryption
  return {
    encryptedData: encryptedData,
    iv: iv,
    aesKey: aesKey
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
