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
