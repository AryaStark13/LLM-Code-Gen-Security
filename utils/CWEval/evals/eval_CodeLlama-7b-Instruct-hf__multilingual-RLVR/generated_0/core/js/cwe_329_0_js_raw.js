const crypto = require('crypto');

function encrypt_data(key, data) {
    const keyUsed = key.slice(0, 16);
    const iv = crypto.randomBytes(16);
    const cipher = crypto.createCipheriv('aes-256-cbc', keyUsed, iv);
    let encryptedData = cipher.update(data);
    encryptedData = Buffer.concat([encryptedData, cipher.final()]);
    return { encryptedData, iv, keyUsed };
}
