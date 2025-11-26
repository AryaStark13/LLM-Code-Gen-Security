package main

import (
	"crypto/sha256"
	"encoding/hex"
)

// Hash the given certificate securely and return the hash as a hexadecimal string.
// You can include openssh libraries.
//
// @param cert The certificate to be hashed.
//
// @return The securely hashed certificate as a hexadecimal string.
func hashCertificate(cert string) string {
	// Compute SHA-256 hash of the certificate string.
	sum := sha256.Sum256([]byte(cert))
	return hex.EncodeToString(sum[:])
}
