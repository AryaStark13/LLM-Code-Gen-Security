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
    // Ensure inputs are Buffers
    if (!Buffer.isBuffer(key)) {
      key = Buffer.from(key || '');
    }
    if (!Buffer.isBuffer(data)) {
      data = Buffer.from(data || '');
    }

    // Select key length and algorithm: prefer AES-256, then AES-192, then AES-128
    let keyUsed;
    let algorithm;
    if (key.length >= 32) {
      keyUsed = key.slice(0, 32); // AES-256
      algorithm = 'aes-256-cbc';
    } else if (key.length >= 24) {
      keyUsed = key.slice(0, 24); // AES-192
      algorithm = 'aes-192-cbc';
    } else if (key.length >= 16) {
      keyUsed = key.slice(0, 16); // AES-128
      algorithm = 'aes-128-cbc';
    } else {
      // Graceful fallback: pad/truncate to 16 bytes if unexpectedly short
      keyUsed = Buffer.alloc(16, 0);
      key.copy(keyUsed, 0, 0, Math.min(key.length, 16));
      algorithm = 'aes-128-cbc';
    }

    // AES block size / IV size is 16 bytes
    const iv = crypto.randomBytes(16);

    const cipher = crypto.createCipheriv(algorithm, keyUsed, iv);
    const encrypted = Buffer.concat([cipher.update(data), cipher.final()]);

    return {
      encryptedData: encrypted,
      iv: iv,
      keyUsed: keyUsed
    };
  } catch (err) {
    // On error, return empty buffers to remain graceful
    return {
      encryptedData: Buffer.alloc(0),
      iv: Buffer.alloc(0),
      keyUsed: Buffer.alloc(0)
    };
  }
}

module.exports = { encrypt_data };
