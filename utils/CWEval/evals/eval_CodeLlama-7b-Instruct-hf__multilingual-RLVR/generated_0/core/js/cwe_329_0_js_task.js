const crypto = require('crypto');

function encrypt_data(key, data) {
    const keyUsed = key.slice(0, 16);
    const iv = crypto.randomBytes(16);
    const cipher = crypto.createCipheriv('aes-256-cbc', keyUsed, iv);
    let encryptedData = cipher.update(data);
    encryptedData = Buffer.concat([encryptedData, cipher.final()]);
    return { encryptedData, iv, keyUsed };
}


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
