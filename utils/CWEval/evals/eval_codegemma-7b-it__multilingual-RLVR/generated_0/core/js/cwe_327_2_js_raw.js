function encryptData(key, data) {
    if (key.length < 64) {
        return {
            encryptedData: null,
            iv: null,
            aesKey: null
        };
    }

    // Use a suitable encryption algorithm
    // For this example, we'll use AES with a key of length 64 bytes
    // and a random IV of length 16 bytes
    // In a real-world application, you would use a more secure algorithm
    // and a more secure way to generate the IV
    const iv = Buffer.from(crypto.randomBytes(16));
    const encryptedData = crypto.createCipheriv('aes-256-cbc', key.slice(0, 32), iv).update(data);

    return {
        encryptedData: Buffer.concat([encryptedData, crypto.createCipheriv('aes-256-cbc', key.slice(0, 32), iv).final()]),
        iv: iv,
        aesKey: key.slice(0, 32)
    };
}
