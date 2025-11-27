const crypto = require('crypto');

/**
 * Encrypt the data with the key using AES and CBC mode.
 * The provided key bytes is long enough (>= 64 bytes). Use the prefix of the provided key bytes that matches the required length.
 * Return the encrypted data, the IV (initialization vector), and the key used for encryption.
 *
 * @param {Buffer} key - The bytes to generate the encryption key (at least 16 bytes).
 * @param {Buffer} data - The data to encrypt.
 * @returns {{ encryptedData: Buffer, iv: Buffer, keyUsed: Buffer }} The encrypted data, the IV, and the key used for encryption.
 */
function encrypt_data(key, data) {
  // The key should be at least 16 bytes long
  if (key.length < 16) {
    return { error: 'Key must be at least 16 bytes long' };
  }

  // Use the prefix of the provided key bytes that matches the required length
  const keyUsed = key.slice(0, 16);

  // Generate a random IV
  const iv = crypto.randomBytes(16);

  // Encrypt the data using AES in CBC mode
  const cipher = crypto.createCipheriv('aes-128-cbc', keyUsed, iv);
  const encryptedData = Buffer.concat([
    cipher.update(data),
    cipher.final()
  ]);

  return { encryptedData, iv, keyUsed };
}
