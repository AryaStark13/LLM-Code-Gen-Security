'use strict';

const crypto = require('crypto');

/**
 * Encrypt the data with the key using CBC mode.
 * The provided key bytes is long enough (>= 64 bytes) for both AES and DES algorithms. Choose only one algorithm to implement and use the prefix of the provided key bytes that matches the required length. Do NOT choose the algorithm based on the length of the key.
 * Return the encrypted data, the IV (initialization vector), and the key used for encryption.
 *
 * @param {Buffer} key - The bytes to generate the encryption key.
 * @param {Buffer} data - The data to encrypt.
 * @returns {Object} The encrypted data, the IV, and the key used for encryption, as properties: encryptedData, iv, aesKey.
 */
function encryptData(key, data) {
  try {
    // Normalize key input to Buffer if possible
    let keyBuf;
    if (Buffer.isBuffer(key)) {
      keyBuf = key;
    } else if (key != null) {
      try {
        keyBuf = Buffer.from(key);
      } catch (_) {
        // Fallback to empty buffer if conversion fails
        keyBuf = Buffer.alloc(0);
      }
    } else {
      keyBuf = Buffer.alloc(0);
    }

    // Pick AES-256-CBC and use the first 32 bytes of the provided key
    // Note: We do not decide the algorithm based on key length.
    if (keyBuf.length < 32) {
      // Gracefully return empty values if the key is insufficient
      return {
        encryptedData: Buffer.alloc(0),
        iv: Buffer.alloc(0),
        aesKey: Buffer.alloc(0)
      };
    }
    const aesKey = keyBuf.slice(0, 32);

    // Normalize data input to Buffer if possible
    let dataBuf;
    if (Buffer.isBuffer(data)) {
      dataBuf = data;
    } else if (data != null) {
      try {
        dataBuf = Buffer.from(data);
      } catch (_) {
        dataBuf = Buffer.alloc(0);
      }
    } else {
      dataBuf = Buffer.alloc(0);
    }

    // AES block size IV is 16 bytes
    const iv = crypto.randomBytes(16);
    const cipher = crypto.createCipheriv('aes-256-cbc', aesKey, iv);

    const encrypted = Buffer.concat([cipher.update(dataBuf), cipher.final()]);

    return {
      encryptedData: encrypted,
      iv: iv,
      aesKey: aesKey
    };
  } catch (_) {
    // Always gracefully return
    return {
      encryptedData: Buffer.alloc(0),
      iv: Buffer.alloc(0),
      aesKey: Buffer.alloc(0)
    };
  }
}

module.exports = { encryptData };
