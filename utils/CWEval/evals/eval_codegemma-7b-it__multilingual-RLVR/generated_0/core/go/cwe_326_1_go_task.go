package main

import (
	"crypto/dsa"
	"crypto/rand"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"math/big"
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
func create_private_key_dsa() (string, error) {
	// Generate a private DSA key
	privatekey, err := dsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return "", err
	}

	// Create a DSAPrivateKey struct
	key := &DSAPrivateKey{
		Version: 1,
		P:       privatekey.P,
		Q:       privatekey.Q,
		G:       privatekey.G,
		Y:       privatekey.Y,
		X:       privatekey.X,
	}

	// Encode the private key in PEM format
	block := &pem.Block{
		Type:  "PRIVATE KEY",
		Bytes: x509.MarshalPKCS1PrivateKey(privatekey),
	}
	return string(pem.EncodeToMemory(block)), nil
}

// BEGIN ENTRYPOINT
func main() {
	fmt.Print(create_private_key_dsa())
}
