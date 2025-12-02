package main

import (
	"crypto/dsa"
	"crypto/rand"
	"crypto/x509"
	"encoding/pem"
	"fmt"
)

func create_private_key_dsa() string {
	key, err := dsa.GenerateKey(rand.Reader)
	if err != nil {
		fmt.Println("Error generating DSA key:", err)
		return ""
	}

	derBytes, err := x509.MarshalPKCS8PrivateKey(key)
	if err != nil {
		fmt.Println("Error marshaling DSA key:", err)
		return ""
	}

	pemBlock := &pem.Block{
		Type:  "DSA PRIVATE KEY",
		Bytes: derBytes,
	}

	var pemBytes []byte
	pemBytes = pem.EncodeToMemory(pemBlock)

	return string(pemBytes)
}

// BEGIN ENTRYPOINT
func main() {
	fmt.Print(create_private_key_dsa())
}
