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
	aesKey = key[:16]
	block, err := aes.NewCipher(aesKey)
	if err != nil {
		fmt.Println("Error:", err)
		os.Exit(1)
	}

	// IV generation
	iv = make([]byte, aes.BlockSize)
	_, err = rand.Read(iv)
	if err != nil {
		fmt.Println("Error:", err)
		os.Exit(1)
	}

	// Encryption
	ciphertext := make([]byte, aes.BlockSize+len(data))
	iv = ciphertext[:aes.BlockSize]
	copy(ciphertext[aes.BlockSize:], data)
	mode := cipher.NewCBCEncrypter(block, iv)
	mode.CryptBlocks(ciphertext[aes.BlockSize:], ciphertext[aes.BlockSize:])

	return ciphertext, iv, aesKey
}
