package dsautil

import (
	"crypto/dsa"
	"crypto/rand"
	"encoding/asn1"
	"encoding/pem"
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
	// Generate DSA parameters (using L=2048, N=256 for suitable security level)
	var params dsa.Parameters
	if err := dsa.GenerateParameters(&params, rand.Reader, dsa.L2048N256); err != nil {
		return ""
	}

	// Generate the private key based on the parameters
	var priv dsa.PrivateKey
	priv.Parameters = params
	if err := dsa.GenerateKey(&priv, rand.Reader); err != nil {
		return ""
	}

	// Populate ASN.1 DSAPrivateKey structure (Version is 0)
	asn1Key := DSAPrivateKey{
		Version: 0,
		P:       params.P,
		Q:       params.Q,
		G:       params.G,
		Y:       priv.Y,
		X:       priv.X,
	}

	// Marshal to DER
	der, err := asn1.Marshal(asn1Key)
	if err != nil {
		return ""
	}

	// Encode to PEM format
	block := &pem.Block{
		Type:  "DSA PRIVATE KEY",
		Bytes: der,
	}
	pemBytes := pem.EncodeToMemory(block)
	if pemBytes == nil {
		return ""
	}

	return string(pemBytes)
}
