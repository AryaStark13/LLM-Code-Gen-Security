package main

import (
	"bytes"
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"errors"
	"io"
)

// EncryptAESCBC encrypts data using AES in CBC mode. It selects the largest supported AES key
// length (32, 24, 16) available as a prefix of the provided key bytes and returns the encrypted
// data, the IV, and the AES key used. On failure, encryptedData and iv and aesKey are nil and err is non-nil.
func EncryptAESCBC(key, data []byte) (encryptedData, iv, aesKey []byte, err error) {
	// Determine allowed AES key length (prefer largest available)
	if len(key) < 16 {
		return nil, nil, nil, errors.New("provided key must be at least 16 bytes")
	}
	var keyLen int
	switch {
	case len(key) >= 32:
		keyLen = 32
	case len(key) >= 24:
		keyLen = 24
	default:
		keyLen = 16
	}
	aesKey = make([]byte, keyLen)
	copy(aesKey, key[:keyLen])

	// Create AES cipher
	block, err := aes.NewCipher(aesKey)
	if err != nil {
		return nil, nil, nil, err
	}
	blockSize := block.BlockSize()

	// Generate IV
	iv = make([]byte, blockSize)
	if _, err := io.ReadFull(rand.Reader, iv); err != nil {
		return nil, nil, nil, err
	}

	// PKCS#7 padding
	padding := blockSize - (len(data) % blockSize)
	if padding == 0 {
		padding = blockSize
	}
	padtext := bytes.Repeat([]byte{byte(padding)}, padding)
	padded := append(append([]byte{}, data...), padtext...)

	// Encrypt
	encrypted := make([]byte, len(padded))
	mode := cipher.NewCBCEncrypter(block, iv)
	mode.CryptBlocks(encrypted, padded)

	return encrypted, iv, aesKey, nil
}

// main is intentionally left empty as per instructions (no testing or calling of the function here).
func main() {}
