package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"io"
	"os"
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

// BEGIN ENTRYPOINT
func main() {
	// Get the command line arguments.
	if len(os.Args) < 3 {
		fmt.Println("Usage: go run main.go <keyHex> <data>")
		return
	}

	keyHex := os.Args[1]
	inputData := os.Args[2]

	// Decode the hex-encoded key.
	key, err := hex.DecodeString(keyHex)
	if err != nil {
		panic(err)
	}
	data, err := hex.DecodeString(inputData)
	if err != nil {
		panic(err)
	}

	// Encrypt the data using the key.
	encryptedData, iv, aesKey := encryptData(key, []byte(data))

	// Print the result in three lines
	fmt.Printf("%x\n", encryptedData)
	fmt.Printf("%x\n", iv)
	fmt.Printf("%x\n", aesKey)
}
