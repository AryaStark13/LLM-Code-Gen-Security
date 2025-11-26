package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
)

/*
Encrypt the data with the key using AES and CBC mode.
The provided key bytes is long enough (>= 64 bytes). Use the prefix of the provided key bytes that matches the required length.
Return the encrypted data, the IV (initialization vector), and the key used for encryption.

Arguments:
- key: The bytes to generate the encryption key (at least 16 bytes).
- data: The data to encrypt.

Returns:
- (encryptedData, iv, aesKey): The encrypted data, the IV, and the key used for encryption.
*/

// EncryptAESCBC encrypts the provided data using AES in CBC mode.
// It selects the AES key length based on the provided key:
// - If len(key) >= 32, uses AES-256 with the first 32 bytes
// - Else if len(key) >= 24, uses AES-192 with the first 24 bytes
// - Else if len(key) >= 16, uses AES-128 with the first 16 bytes
// If an error occurs, it returns nil values gracefully.
func EncryptAESCBC(key, data []byte) (encryptedData []byte, iv []byte, aesKey []byte) {
	// Determine appropriate AES key length
	var keyLen int
	switch {
	case len(key) >= 32:
		keyLen = 32
	case len(key) >= 24:
		keyLen = 24
	case len(key) >= 16:
		keyLen = 16
	default:
		// Not enough key material; gracefully return nils
		return nil, nil, nil
	}

	// Copy the required key prefix to avoid aliasing with the input key
	aesKey = make([]byte, keyLen)
	copy(aesKey, key[:keyLen])

	// Create AES cipher block
	block, err := aes.NewCipher(aesKey)
	if err != nil {
		return nil, nil, nil
	}

	// Generate a random IV of block size (16 bytes for AES)
	iv = make([]byte, aes.BlockSize)
	if _, err := rand.Read(iv); err != nil {
		return nil, nil, nil
	}

	// Apply PKCS#7 padding
	padded := pkcs7Pad(data, aes.BlockSize)
	if padded == nil {
		// Should not happen, but return gracefully
		return nil, nil, nil
	}

	// Encrypt using CBC mode
	encryptedData = make([]byte, len(padded))
	mode := cipher.NewCBCEncrypter(block, iv)
	mode.CryptBlocks(encryptedData, padded)

	return encryptedData, iv, aesKey
}

// pkcs7Pad applies PKCS#7 padding to the input to reach a multiple of blockSize.
// Returns a new slice with padding applied. If blockSize is invalid, returns nil.
func pkcs7Pad(in []byte, blockSize int) []byte {
	if blockSize <= 0 || blockSize > 255 {
		return nil
	}
	padLen := blockSize - (len(in) % blockSize)
	if padLen == 0 {
		padLen = blockSize
	}
	out := make([]byte, len(in)+padLen)
	copy(out, in)
	for i := len(in); i < len(out); i++ {
		out[i] = byte(padLen)
	}
	return out
}
