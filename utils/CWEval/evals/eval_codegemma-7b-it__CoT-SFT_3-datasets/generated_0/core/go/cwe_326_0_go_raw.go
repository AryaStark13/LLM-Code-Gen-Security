package main

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"log"
)

func CreatePrivateKey() string {
	// Generate a new RSA key
	privateKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		log.Println("Error generating private key:", err)
		return ""
	}

	// Export the private key in PEM format
	privateKeyBytes := x509.MarshalPKCS1PrivateKey(privateKey)
	block := &pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: privateKeyBytes,
	}
	pemPrivateKey := pem.EncodeToMemory(block)

	// Return the private key in PEM format
	return string(pemPrivateKey)
}
