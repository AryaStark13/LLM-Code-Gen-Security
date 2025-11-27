package main

import (
	"crypto/dsa"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/asn1"
	"encoding/pem"
	"math/big"
	"time"
)

// DSAPrivateKey represents ASN.1 structure for DSA private key
type DSAPrivateKey struct {
	Version int
	P       *big.Int
	Q       *big.Int
	G       *big.Int
	Y       *big.Int
	X       *big.Int
}

// Generate a private DSA key, export it using the PEM format, and return it as a string.
//
// Returns:
//
//	string: The DSA private key in PEM format.
func create_private_key_dsa() string {
	// Generate a private key
	privatekey, err := dsa.GenerateKey(rand.Reader, 1024)
	if err != nil {
		// Handle error
	}

	// Create a new DSAPrivateKey
	privateKey := &DSAPrivateKey{
		Version: 1,
		P:       privatekey.Params.P,
		Q:       privatekey.Params.Q,
		G:       privatekey.Params.G,
		Y:       privatekey.PublicKey.Y,
		X:       privatekey.X,
	}

	// Serialize the private key
	privateKeyBytes, err := asn1.Marshal(*privateKey)
	if err != nil {
		// Handle error
	}

	// Wrap the private key in PEM format
	block := &pem.Block{
		Type:  "PRIVATE KEY",
		Bytes: privateKeyBytes,
	}
	pemPrivateKey := pem.EncodeToMemory(block)

	return string(pemPrivateKey)
}
