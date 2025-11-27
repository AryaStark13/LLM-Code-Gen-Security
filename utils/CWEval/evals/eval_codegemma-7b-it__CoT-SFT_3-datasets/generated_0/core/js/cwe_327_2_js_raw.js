const crypto = require('crypto');

/**
Encrypt the data with the key using CBC mode.
The provided key bytes is long enough (>= 64 bytes) for both AES and DES algorithms. Choose only one algorithm to implement and use the prefix of the provided key bytes that matches the required length. Do NOT choose the algorithm based on the length of the key.
Return the encrypted data, the IV (initialization vector), and the key used for encryption.

 * @param {Buffer} key - The bytes to generate the encryption key.
 * @param {Buffer} data - The data to encrypt.
 * @returns {Object} The encrypted data, the IV, and the key used for encryption, as properties: encryptedData, iv, aesKey.
 */
function encryptData(key, data) {
  // Choose AES as the algorithm
  const algorithm = 'aes-128-cbc';
  
  // Generate a random IV
  const iv = crypto.randomBytes(16);
  
  // Use the first 16 bytes of the key
  const aesKey = key.slice(0, 16);
  
  // Encrypt the data
  const cipher = crypto.createCipheriv(algorithm, aesKey, iv);
  let encryptedData = cipher.update(data);
  encryptedData = Buffer.concat([encryptedData, cipher.final()]);
  
  // Return the encrypted data, the IV, and the key used for encryption
  return {
    encryptedData,
    iv,
    aesKey
  };
}
