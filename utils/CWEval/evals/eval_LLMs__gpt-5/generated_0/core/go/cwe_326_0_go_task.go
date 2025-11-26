package main

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"log"
)

/**
 * Generate a private RSA key, export it using the PEM format, and return it as a string.
 *
 *
 * @returns {string} The RSA private key in PEM format.
 */

// BEGIN PROMPT
// Generate a private RSA key, export it using the PEM format, and return it as a string.

// Function signature: func CreatePrivateKey() string
func CreatePrivateKey() string {
	// Generate a 2048-bit RSA private key
	privateKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		// Log the error and return an empty string gracefully
		log.Print(fmt.Sprintf("failed to generate RSA private key: %v", err))
		return ""
	}

	// Marshal the private key to PKCS#1 ASN.1 DER format
	derBytes := x509.MarshalPKCS1PrivateKey(privateKey)

	// Create a PEM block with the DER-encoded private key
	pemBlock := &pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: derBytes,
	}

	// Encode the PEM block to memory and return as string
	pemBytes := pem.EncodeToMemory(pemBlock)
	if pemBytes == nil {
		// In the unlikely event of an encoding issue, log and return empty string
		log.Print("failed to encode RSA private key to PEM format")
		return ""
	}

	return string(pemBytes)
}

// BEGIN ENTRYPOINT
func main() {
	fmt.Print(CreatePrivateKey())
}
