package cbcenc

import (
	"bytes"
	"crypto/aes"
	"crypto/cipher"
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
// - The encrypted data, the IV, and the key used for encryption. (encryptedData, iv, aesKey, error)
func main(key []byte, data []byte) ([]byte, []byte, []byte, error) {
	// Choose AES-256 for encryption, use the first 32 bytes of the provided key.
	const aesKeyLen = 32

	if len(key) < aesKeyLen {
		return nil, nil, nil, errors.New("provided key must be at least 32 bytes for AES-256")
	}

	// Use the first 32 bytes as the AES key (copy to avoid aliasing the original slice).
	aesKey := make([]byte, aesKeyLen)
	copy(aesKey, key[:aesKeyLen])

	// Initialize AES cipher block.
	block, err := aes.NewCipher(aesKey)
	if err != nil {
		return nil, nil, nil, err
	}

	blockSize := block.BlockSize() // AES block size is 16 bytes.

	// Generate a random IV.
	iv := make([]byte, blockSize)
	if _, err := io.ReadFull(rand.Reader, iv); err != nil {
		return nil, nil, nil, err
	}

	// PKCS#7 pad the data to a multiple of the block size.
	padded, err := pkcs7Pad(data, blockSize)
	if err != nil {
		return nil, nil, nil, err
	}

	// Perform CBC encryption.
	ciphertext := make([]byte, len(padded))
	encrypter := cipher.NewCBCEncrypter(block, iv)
	encrypter.CryptBlocks(ciphertext, padded)

	return ciphertext, iv, aesKey, nil
}

// pkcs7Pad pads the input using PKCS#7 to the given block size.
func pkcs7Pad(data []byte, blockSize int) ([]byte, error) {
	if blockSize <= 0 || blockSize > 255 {
		return nil, errors.New("invalid block size for PKCS#7 padding")
	}

	padLen := blockSize - (len(data) % blockSize)
	if padLen == 0 {
		padLen = blockSize
	}

	padding := bytes.Repeat([]byte{byte(padLen)}, padLen)
	// Use append on a copy to avoid modifying the original data slice.
	out := make([]byte, 0, len(data)+padLen)
	out = append(out, data...)
	out = append(out, padding...)
	return out, nil
}
