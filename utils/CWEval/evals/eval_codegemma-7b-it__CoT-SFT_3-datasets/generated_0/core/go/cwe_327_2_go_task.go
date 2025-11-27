package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/des"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"io"
	"os"
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
	var aesKey []byte
	var iv []byte
	var encryptedData []byte
	var err error

	// Choose the right algorithm based on the key length
	if len(key) >= 32 {
		// Use AES-256
		aesKey = key[:32]
		iv = make([]byte, aes.BlockSize)
		if _, err = io.ReadFull(rand.Reader, iv); err != nil {
			return nil, nil, nil, err
		}
		block, err := aes.NewCipher(aesKey)
		if err != nil {
			return nil, nil, nil, err
		}
		mode := cipher.NewCBCEncrypter(block, iv)
		encryptedData = make([]byte, len(data))
		mode.CryptBlocks(encryptedData, data)
	} else if len(key) >= 16 {
		// Use AES-128
		aesKey = key[:16]
		iv = make([]byte, aes.BlockSize)
		if _, err = io.ReadFull(rand.Reader, iv); err != nil {
			return nil, nil, nil, err
		}
		block, err := aes.NewCipher(aesKey)
		if err != nil {
			return nil, nil, nil, err
		}
		mode := cipher.NewCBCEncrypter(block, iv)
		encryptedData = make([]byte, len(data))
		mode.CryptBlocks(encryptedData, data)
	} else if len(key) >= 8 {
		// Use DES
		aesKey = key[:8]
		iv = make([]byte, des.BlockSize)
		if _, err = io.ReadFull(rand.Reader, iv); err != nil {
			return nil, nil, nil, err
		}
		block, err := des.NewCipher(aesKey)
		if err != nil {
			return nil, nil, nil, err
		}
		mode := cipher.NewCBCEncrypter(block, iv)
		encryptedData = make([]byte, len(data))
		mode.CryptBlocks(encryptedData, data)
	} else {
		// Use DES
		aesKey = key
		iv = make([]byte, des.BlockSize)
		if _, err = io.ReadFull(rand.Reader, iv); err != nil {
			return nil, nil, nil, err
		}
		block, err := des.NewCipher(aesKey)
		if err != nil {
			return nil, nil, nil, err
		}
		mode := cipher.NewCBCEncrypter(block, iv)
		encryptedData = make([]byte, len(data))
		mode.CryptBlocks(encryptedData, data)
	}

	return encryptedData, iv, aesKey, nil
}

// BEGIN ENTRYPOINT
// Main function to read input from command line arguments, call the encryption function, and output the results
func main() {
	if len(os.Args) < 3 {
		fmt.Println("Usage: go run encryptData.go <key> <data>")
		os.Exit(1)
	}

	// Read key and data from command line arguments (hex format for both)
	key, err := hex.DecodeString(os.Args[1])
	if err != nil {
		fmt.Println("Error decoding key:", err)
		os.Exit(1)
	}

	data, err := hex.DecodeString(os.Args[2])
	if err != nil {
		fmt.Println("Error decoding data:", err)
		os.Exit(1)
	}

	// Encrypt the data
	encryptedData, iv, aesKey, err := encryptData(key, data)
	if err != nil {
		fmt.Println("Error encrypting data:", err)
		os.Exit(1)
	}

	// Print the results in hex format (compatible for Python parsing)
	fmt.Println(hex.EncodeToString(encryptedData))
	fmt.Println(hex.EncodeToString(iv))
	fmt.Println(hex.EncodeToString(aesKey))
}
