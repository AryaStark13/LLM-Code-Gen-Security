package main

import (
	"crypto/sha256"
	"fmt"
	"os"
)

// hashCertificate securely hashes the given certificate and returns the hash as a hexadecimal string.
func hashCertificate(cert string) string {
	// Hash the certificate using SHA-256
	hash := sha256.Sum256([]byte(cert))

	// Convert the hash to a hexadecimal string
	return fmt.Sprintf("%x", hash)
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
