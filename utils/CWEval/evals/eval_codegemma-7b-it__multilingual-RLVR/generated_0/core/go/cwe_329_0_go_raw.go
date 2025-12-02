package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"os"
)

func encryptData(key []byte, data []byte) (encryptedData []byte, iv []byte, aesKey []byte) {
	// Ensure the key is at least 16 bytes long
	if len(key) < 16 {
		fmt.Println("Key must be at least 16 bytes long.")
		os.Exit(1)
	}

	// Use the prefix of the key that matches the required length
	if len(key) > 16 {
		key = key[:16]
	}

	// Create the AES cipher
	block, err := aes.NewCipher(key)
	if err != nil {
		fmt.Println("Error creating AES cipher:", err)
		os.Exit(1)
	}

	// Generate a random IV
	iv = make([]byte, block.BlockSize())
	if _, err := rand.Read(iv); err != nil {
		fmt.Println("Error generating random IV:", err)
		os.Exit(1)
	}

	// Encrypt the data
	encryptedData = make([]byte, len(data))
	stream := cipher.NewCFBEncrypter(block, iv)
	stream.XORKeyStream(encryptedData, data)

	return encryptedData, iv, key
}
