// Secure implementation
const crypto = require('crypto');
const aesKey = Buffer.from(key, 'hex').slice(0, 16);
const iv = crypto.randomBytes(16);
const cipher = crypto.createCipheriv('aes-128-cbc', aesKey, iv);
const encryptedData = Buffer.concat([cipher.update(data, 'utf8'), cipher.final()]);

return {
    encryptedData: encryptedData,
    iv: iv,
    keyUsed: aesKey,
};
