'use strict';

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
  // Always return an object with Buffers, even on error.
  const emptyResult = {
    encryptedData: Buffer.alloc(0),
    iv: Buffer.alloc(0),
    aesKey: Buffer.alloc(0)
  };

  try {
    // Validate and normalize inputs to Buffers
    if (!key) return emptyResult;
    if (!data) return emptyResult;

    const keyBuf = Buffer.isBuffer(key) ? key : Buffer.from(key);
    const dataBuf = Buffer.isBuffer(data) ? data : Buffer.from(data);

    // Choose AES-256-CBC (fixed choice). Use the prefix of provided key bytes (32 bytes).
    const AES_KEY_LEN = 32; // 256 bits
    if (keyBuf.length < AES_KEY_LEN) {
      // Defensive: if not enough bytes, return empty result
      return emptyResult;
    }
    const aesKey = keyBuf.slice(0, AES_KEY_LEN);

    // IV for AES-CBC is 16 bytes
    const iv = crypto.randomBytes(16);

    // Create cipher and encrypt
    const cipher = crypto.createCipheriv('aes-256-cbc', aesKey, iv);
    const encryptedParts = [];
    encryptedParts.push(cipher.update(dataBuf));
    encryptedParts.push(cipher.final());
    const encryptedData = Buffer.concat(encryptedParts);

    return {
      encryptedData: encryptedData,
      iv: iv,
      aesKey: aesKey
    };
  } catch (e) {
    // On any error, return empty buffers gracefully
    return emptyResult;
  }
}

module.exports = { encryptData };
