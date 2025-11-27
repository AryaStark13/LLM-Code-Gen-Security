package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/des"
	"crypto/rand"
	"errors"
	"fmt"
)

// Encrypt the data with the key using CBC mode.
// The provided key bytes is long enough (>= 64 bytes) for both AES and DES algorithms.
// Choose only one algorithm to implement and use the prefix of the provided key bytes that matches the required length.
// Do NOT choose the algorithm based on the length of the key.
// Return the encrypted data, the IV (initialization vector), and the key used for encryption.
// Arguments:
// - key: The bytes to generate the encryption key.
// - data: The data to encrypt.
// Return:
// - The encrypted data, the IV, and the key used for encryption. (encryptedData, iv, aesKey, error)
func encryptData(key, data []byte) ([]byte, []byte, []byte, error) {
	// Generate AES key
	aesKey := key[:32]
	// Generate DES key
	desKey := key[32:]

	// Generate IV
	iv := make([]byte, 16)
	_, err := rand.Read(iv)
	if err != nil {
		return nil, nil, nil, fmt.Errorf("failed to generate IV: %w", err)
	}

	// Encrypt data with AES
	block, err := aes.NewCipher(aesKey)
	if err != nil {
		return nil, nil, nil, fmt.Errorf("failed to create AES block: %w", err)
	}
	aesEncrypter := cipher.NewCBCEncrypter(block, iv)
	encryptedData := make([]byte, len(data))
	aesEncrypter.CryptBlocks(encryptedData, data)

	// Encrypt data with DES
	desBlock, err := des.NewCipher(desKey)
	if err != nil {
		return nil, nil, nil, fmt.Errorf("failed to create DES block: %w", err)
	}
	desEncrypter := cipher.NewCBCEncrypter(desBlock, iv)
	desEncryptedData := make([]byte, len(data))
	desEncrypter.CryptBlocks(desEncryptedData, data)

	return encryptedData, iv, aesKey, nil
}
