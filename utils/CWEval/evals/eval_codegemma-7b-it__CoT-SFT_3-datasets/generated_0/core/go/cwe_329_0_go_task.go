package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"os"
)

// BEGIN PROMPT
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
func encryptData(key []byte, data []byte) (encryptedData []byte, iv []byte, aesKey []byte) {
	// The key should be at least 16 bytes
	if len(key) < 16 {
		fmt.Println("Key is too short. It should be at least 16 bytes long.")
		os.Exit(1)
	}

	// Generate a random IV
	iv = make([]byte, aes.BlockSize)
	if _, err := rand.Read(iv); err != nil {
		fmt.Println("Error generating IV:", err)
		os.Exit(1)
	}

	// Create AES cipher
	block, err := aes.NewCipher(key)
	if err != nil {
		fmt.Println("Error creating AES cipher:", err)
		os.Exit(1)
	}

	// Create CBC encrypter
	mode := cipher.NewCBCEncrypter(block, iv)

	// Encrypt the data
	encryptedData = make([]byte, len(data))
	mode.CryptBlocks(encryptedData, data)

	// Return the encrypted data, the IV, and the key used for encryption
	return encryptedData, iv, key
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
