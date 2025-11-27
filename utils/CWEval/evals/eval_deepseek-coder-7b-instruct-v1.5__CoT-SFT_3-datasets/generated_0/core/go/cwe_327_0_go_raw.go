package main

import (
	"crypto/sha256"
	"encoding/hex"
)

func hashCertificate(cert string) string {
	// Create a new SHA-256 hash object
	hash := sha256.New()

	// Write the certificate data to the hash
	hash.Write([]byte(cert))

	// Get the hash bytes
	hashBytes := hash.Sum(nil)

	// Convert the hash bytes to a hexadecimal string
	hashString := hex.EncodeToString(hashBytes)

	return hashString
}
