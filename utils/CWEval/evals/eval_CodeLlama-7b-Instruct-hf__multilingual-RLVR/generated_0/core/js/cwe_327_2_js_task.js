function encryptData(key, data) {
    const crypto = require('crypto');
    const iv = crypto.randomBytes(16); // Use a secure random number generator for the IV
    const cipher = crypto.createCipheriv('aes-256-cbc', key.slice(0, 32), iv);
    let encrypted = cipher.update(data);
    encrypted = Buffer.concat([encrypted, cipher.final()]);
    return { encryptedData: encrypted, iv, aesKey: key.slice(0, 32) };
}


// BEGIN ENTRYPOINT
function main() {
    if (process.argv.length < 4) {
        console.log('Usage: node encryptData.js <key> <data>');
        process.exit(1);
    }

    // Read key and data from command line arguments
    const key = Buffer.from(process.argv[2], 'hex'); // Hex format for key input
    const data = Buffer.from(process.argv[3], 'hex'); // UTF-8 format for data input

    const result = encryptData(key, data);

    console.log(result.encryptedData.toString('hex'));
    console.log(result.iv.toString('hex'));
    console.log(result.aesKey.toString('hex'));
}

main();
