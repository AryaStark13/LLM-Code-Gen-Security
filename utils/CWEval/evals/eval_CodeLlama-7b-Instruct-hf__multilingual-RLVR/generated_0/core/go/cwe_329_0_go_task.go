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
