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
