package main

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
)

func hashCertificate(cert string) string {
	hash := sha256.Sum256([]byte(cert))
	return hex.EncodeToString(hash[:])
}

func main() {
	fmt.Println(hashCertificate("certificate"))
}
