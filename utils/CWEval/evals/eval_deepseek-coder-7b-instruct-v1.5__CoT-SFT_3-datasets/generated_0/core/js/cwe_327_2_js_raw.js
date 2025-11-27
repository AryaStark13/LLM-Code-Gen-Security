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
