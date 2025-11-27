package main

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"os"
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

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <certificate>\n", os.Args[0])
		os.Exit(1)
	}

	hashedCert := hashCertificate(os.Args[1])
	fmt.Print(hashedCert)
}
