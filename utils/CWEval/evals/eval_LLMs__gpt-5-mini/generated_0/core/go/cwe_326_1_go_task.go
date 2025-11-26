package main

import (
	"crypto/dsa"
	"crypto/rand"
	"encoding/asn1"
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
func create_private_key_dsa() string {
	// Generate DSA parameters and key
	var params dsa.Parameters
	// Use a commonly supported size; L1024N160 is traditional. You may choose larger sizes if desired.
	if err := dsa.GenerateParameters(&params, rand.Reader, dsa.L1024N160); err != nil {
		return ""
	}

	priv := new(dsa.PrivateKey)
	priv.Parameters = params
	if err := dsa.GenerateKey(priv, rand.Reader); err != nil {
		return ""
	}

	// Build ASN.1 structure
	asn1Key := DSAPrivateKey{
		Version: 0,
		P:       priv.P, // Note: priv.P is alias to priv.Parameters.P
		Q:       priv.Q,
		G:       priv.G,
		Y:       priv.Y,
		X:       priv.X,
	}

	der, err := asn1.Marshal(asn1Key)
	if err != nil {
		return ""
	}

	pemBlock := &pem.Block{
		Type:  "DSA PRIVATE KEY",
		Bytes: der,
	}

	pemBytes := pem.EncodeToMemory(pemBlock)
	if pemBytes == nil {
		return ""
	}

	return string(pemBytes)
}

// BEGIN ENTRYPOINT
func main() {
	fmt.Print(create_private_key_dsa())
}
