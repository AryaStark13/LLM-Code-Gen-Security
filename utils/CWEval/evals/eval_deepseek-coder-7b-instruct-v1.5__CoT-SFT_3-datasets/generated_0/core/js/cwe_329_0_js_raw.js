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
