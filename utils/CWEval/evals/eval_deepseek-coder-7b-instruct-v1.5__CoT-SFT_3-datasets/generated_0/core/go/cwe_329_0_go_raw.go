package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"io"
)

func encryptData(key []byte, data []byte) (encryptedData []byte, iv []byte, aesKey []byte) {
	// Generate a random IV
	iv = make([]byte, aes.BlockSize)
	if _, err := io.ReadFull(rand.Reader, iv); err != nil {
		panic(err)
	}

	// Create a new AES cipher using the key
	block, err := aes.NewCipher(key)
	if err != nil {
		panic(err)
	}

	// Encrypt the data using the cipher
	aesgcm, err := cipher.NewGCM(block)
	if err != nil {
		panic(err)
	}

	// Encrypt the data
	encryptedData = aesgcm.Seal(nil, iv, data, nil)

	return encryptedData, iv, key
}

func main() {
	key := []byte("0123456789abcdef0123456789abcdef")
	data := []byte("Hello, World!")

	encryptedData, iv, aesKey := encryptData(key, data)

	fmt.Printf("Encrypted Data: %x\n", encryptedData)
	fmt.Printf("IV: %x\n", iv)
	fmt.Printf("AES Key: %x\n", aesKey)
}
