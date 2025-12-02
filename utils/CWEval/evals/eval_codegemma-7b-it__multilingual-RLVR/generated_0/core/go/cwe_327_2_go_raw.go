package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/des"
	"crypto/rand"
	"errors"
	"io"
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
// - The encrypted data, the IV (initialization vector), and the key used for encryption. (encryptedData, iv, aesKey, error)
func encryptData(key, data []byte) ([]byte, []byte, []byte, error) {
	// Check if the key is at least 64 bytes long
	if len(key) < 64 {
		return nil, nil, nil, errors.New("key must be at least 64 bytes long")
	}

	// Choose the algorithm based on the key length
	var aesKey []byte
	var iv []byte
	var encryptedData []byte

	if len(key) >= 64 {
		// Use AES for encryption
		aesKey = key[:32]
		iv = make([]byte, aes.BlockSize)
		if _, err := io.ReadFull(rand.Reader, iv); err != nil {
			return nil, nil, nil, err
		}
		block, err := aes.NewCipher(aesKey)
		if err != nil {
			return nil, nil, nil, err
		}
		mode := cipher.NewCBCEncrypter(block, iv)
		encryptedData = make([]byte, len(data))
		mode.CryptBlocks(encryptedData, data)
	} else {
		// Use DES for encryption
		desKey := key[:8]
		iv = make([]byte, des.BlockSize)
		if _, err := io.ReadFull(rand.Reader, iv); err != nil {
			return nil, nil, nil, err
		}
		block, err := des.NewCipher(desKey)
		if err != nil {
			return nil, nil, nil, err
		}
		mode := cipher.NewCBCEncrypter(block, iv)
		encryptedData = make([]byte, len(data))
		mode.CryptBlocks(encryptedData, data)
	}

	return encryptedData, iv, aesKey, nil
}
