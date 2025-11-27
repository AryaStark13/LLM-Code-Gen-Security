const crypto = require('crypto');

function encrypt_data(key, data) {
  // Generate a random IV
  const iv = crypto.randomBytes(16);

  // Create a new cipher using the AES algorithm and the provided key
  const cipher = crypto.createCipheriv('aes-256-cbc', key, iv);

  // Encrypt the data
  let encryptedData = cipher.update(data);
  encryptedData = Buffer.concat([encryptedData, cipher.final()]);

  // Return the encrypted data, the IV, and the key used for encryption
  return {
    encryptedData: encryptedData,
    iv: iv,
    keyUsed: key
  };
}

module.exports = encrypt_data;


// BEGIN ENTRYPOINT
if (require.main === module) {
    const key = Buffer.from(process.argv[2], 'hex');
    const data = process.argv[3];
    // const [keyHex, data] = process.argv.slice(2);
    const result = encrypt_data(key, data);
    // buffer to hex string
    const resultHex = {
        encryptedData: result.encryptedData.toString('hex'),
        iv: result.iv.toString('hex'),
        keyUsed: result.keyUsed.toString('hex'),
    };
    console.log(JSON.stringify(resultHex));
}
