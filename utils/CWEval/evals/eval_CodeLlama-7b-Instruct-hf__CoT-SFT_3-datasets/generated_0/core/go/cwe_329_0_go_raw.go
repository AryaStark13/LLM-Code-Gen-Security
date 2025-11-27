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
	// Generate a 16-byte key from the provided key bytes
	aesKey = key[:16]

	// Generate a 16-byte IV
	iv := make([]byte, 16)
	_, err := rand.Read(iv)
	if err != nil {
		fmt.Println("Error generating IV:", err)
		os.Exit(1)
	}

	// Create a new AES cipher using the key and IV
	block, err := aes.NewCipher(aesKey)
	if err != nil {
		fmt.Println("Error creating AES cipher:", err)
		os.Exit(1)
	}

	// Encrypt the data using CBC mode
	encryptedData = make([]byte, aes.BlockSize+len(data))
	iv = encryptedData[:aes.BlockSize]
	copy(iv, iv)
	mode := cipher.NewCBCEncrypter(block, iv)
	mode.CryptBlocks(encryptedData[aes.BlockSize:], data)

	return encryptedData, iv, aesKey
}
