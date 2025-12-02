function encrypt_data(key, data) {
    // Ensure the key is at least 16 bytes long
    if (key.length < 16) {
        return { error: 'Key must be at least 16 bytes long' };
    }
    
    // Generate a random initialization vector
    const iv = Buffer.from(crypto.randomBytes(16));
    
    // Encrypt the data using AES-128-CBC mode
    const cipher = crypto.createCipheriv('aes-128-cbc', key.slice(0, 16), iv);
    const encryptedData = Buffer.concat([cipher.update(data), cipher.final()]);
    
    // Return the encrypted data, the IV, and the key used for encryption
    return { encryptedData, iv, keyUsed: key.slice(0, 16) };
}
