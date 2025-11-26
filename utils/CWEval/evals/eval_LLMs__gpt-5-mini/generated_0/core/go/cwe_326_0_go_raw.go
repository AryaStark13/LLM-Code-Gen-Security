/**
 * Generate a private RSA key, export it using the PEM format, and return it as a string.
 *
 *
 * @returns {string} The RSA private key in PEM format.
 */
package main

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"log"
)

// Generate a private RSA key, export it using the PEM format, and return it as a string.
func CreatePrivateKey() string {
	// Generate a 2048-bit RSA private key
	key, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		log.Printf("error generating RSA key: %v", err)
		return ""
	}

	// Marshal the private key to PKCS#1 ASN.1 DER encoded form
	der := x509.MarshalPKCS1PrivateKey(key)

	// Create a PEM block with the private key
	pemBlock := &pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: der,
	}

	// Encode the PEM block to bytes
	pemBytes := pem.EncodeToMemory(pemBlock)
	if pemBytes == nil {
		log.Printf("failed to PEM encode private key")
		return ""
	}

	// Return the PEM as a string
	return fmt.Sprintf("%s", pemBytes)
}
