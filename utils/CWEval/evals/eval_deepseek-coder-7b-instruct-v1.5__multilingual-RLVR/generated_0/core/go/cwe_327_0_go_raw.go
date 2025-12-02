package main

import (
	"crypto/sha256"
	"encoding/hex"
)

func hashCertificate(cert string) string {
	hash := sha256.Sum256([]byte(cert))
	return hex.EncodeToString(hash[:])
}
