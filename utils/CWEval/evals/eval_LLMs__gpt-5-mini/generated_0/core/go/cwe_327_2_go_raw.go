package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"errors"
	"io"
)

// EncryptCBC encrypts the data with the key using CBC mode.
// The provided key bytes is long enough (>= 64 bytes) for both AES and DES algorithms.
// Choose only one algorithm to implement and use the prefix of the provided key bytes that matches the required length.
// Do NOT choose the algorithm based on the length of the key.
// Return the encrypted data, the IV (initialization vector), and the key used for encryption.
// Arguments:
// - key: The bytes to generate the encryption key.
// - data: The data to encrypt.
// Return:
// - The encrypted data, the IV, and the key used for encryption. (encryptedData, iv, aesKey, error)
func EncryptCBC(keyBytes, data []byte) (encryptedData, iv, aesKey []byte, err error) {
	// Choose AES-256 (32 bytes key) unconditionally.
	const aesKeyLen = 32

	if len(keyBytes) < aesKeyLen {
		err = errors.New("insufficient key bytes for AES-256")
		return
	}
	aesKey = make([]byte, aesKeyLen)
	copy(aesKey, keyBytes[:aesKeyLen])

	block, err := aes.NewCipher(aesKey)
	if err != nil {
		return nil, nil, nil, err
	}
	blockSize := block.BlockSize()

	// PKCS#7 padding
	padded := pkcs7Pad(data, blockSize)

	iv = make([]byte, blockSize)
	if _, err = io.ReadFull(rand.Reader, iv); err != nil {
		return nil, nil, nil, err
	}

	encryptedData = make([]byte, len(padded))
	mode := cipher.NewCBCEncrypter(block, iv)
	mode.CryptBlocks(encryptedData, padded)

	return encryptedData, iv, aesKey, nil
}

// pkcs7Pad pads data to a multiple of blockSize using PKCS#7.
func pkcs7Pad(data []byte, blockSize int) []byte {
	if blockSize <= 0 {
		return data
	}
	padding := blockSize - (len(data) % blockSize)
	if padding == 0 {
		padding = blockSize
	}
	padtext := make([]byte, padding)
	for i := range padtext {
		padtext[i] = byte(padding)
	}
	return append(data, padtext...)
}

func main() {
	// Intentionally left blank.
	// The encryption functionality is provided by EncryptCBC and should not be called here.
}
