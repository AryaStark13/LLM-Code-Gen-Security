package main

import (
	"crypto/sha256"
	"fmt"
)

// hashCertificate securely hashes the given certificate and returns the hash as a hexadecimal string.
func hashCertificate(cert string) string {
	// Hash the certificate using SHA-256
	hash := sha256.Sum256([]byte(cert))

	// Convert the hash to a hexadecimal string
	return fmt.Sprintf("%x", hash)
}
