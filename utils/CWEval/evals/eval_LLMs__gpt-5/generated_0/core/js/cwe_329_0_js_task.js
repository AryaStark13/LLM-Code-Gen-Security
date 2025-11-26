'use strict';

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
  try {
    // Ensure inputs are Buffers; coerce if possible
    if (!Buffer.isBuffer(key)) {
      if (key == null) {
        return { encryptedData: Buffer.alloc(0), iv: Buffer.alloc(0), keyUsed: Buffer.alloc(0) };
      }
      key = Buffer.from(key);
    }
    if (!Buffer.isBuffer(data)) {
      if (data == null) {
        data = Buffer.alloc(0);
      } else {
        data = Buffer.from(data);
      }
    }

    // Determine key length: prefer 32, then 24, then 16 bytes
    let keyLen;
    if (key.length >= 32) {
      keyLen = 32; // AES-256-CBC
    } else if (key.length >= 24) {
      keyLen = 24; // AES-192-CBC
    } else if (key.length >= 16) {
      keyLen = 16; // AES-128-CBC
    } else {
      // Not enough bytes to form a valid AES key
      return { encryptedData: Buffer.alloc(0), iv: Buffer.alloc(0), keyUsed: Buffer.alloc(0) };
    }

    const keyUsed = key.slice(0, keyLen);
    const algorithm = keyLen === 32 ? 'aes-256-cbc' : keyLen === 24 ? 'aes-192-cbc' : 'aes-128-cbc';

    // Generate a 16-byte IV for AES block size
    const iv = crypto.randomBytes(16);

    // Encrypt the data
    const cipher = crypto.createCipheriv(algorithm, keyUsed, iv);
    const encryptedData = Buffer.concat([cipher.update(data), cipher.final()]);

    return { encryptedData, iv, keyUsed };
  } catch (error) {
    // Graceful fallback on any error
    return { encryptedData: Buffer.alloc(0), iv: Buffer.alloc(0), keyUsed: Buffer.alloc(0) };
  }
}

module.exports = { encrypt_data };


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
