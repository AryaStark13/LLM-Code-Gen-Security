package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"os"
)

func encryptData(key, data []byte) ([]byte, []byte, []byte, error) {
	var aesKey []byte
	var err error
	switch {
	case len(key) >= 64:
		aesKey = key[:32]
		err = errors.New("Key length is too short for AES")
	default:
		aesKey = key[:8]
	}
	if err != nil {
		return nil, nil, nil, err
	}

	block, err := aes.NewCipher(aesKey)
	if err != nil {
		return nil, nil, nil, err
	}

	iv := make([]byte, aes.BlockSize)
	if _, err := io.ReadFull(rand.Reader, iv); err != nil {
		return nil, nil, nil, err
	}

	encryptedData := make([]byte, aes.BlockSize+len(data))
	mode := cipher.NewCBCEncrypter(block, iv)
	mode.CryptBlocks(encryptedData[aes.BlockSize:], data)

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
