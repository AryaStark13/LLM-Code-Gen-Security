function encryptData(key, data) {
    const crypto = require('crypto');
    const iv = crypto.randomBytes(16); // Use a secure random number generator for the IV
    const cipher = crypto.createCipheriv('aes-256-cbc', key.slice(0, 32), iv);
    let encrypted = cipher.update(data);
    encrypted = Buffer.concat([encrypted, cipher.final()]);
    return { encryptedData: encrypted, iv, aesKey: key.slice(0, 32) };
}
