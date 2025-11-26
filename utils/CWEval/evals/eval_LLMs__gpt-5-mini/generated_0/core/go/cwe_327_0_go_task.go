package main

import (
	"crypto/sha256"
	"crypto/x509"
	"encoding/hex"
	"encoding/pem"
	"fmt"
	"os"
	"strings"

	"golang.org/x/crypto/ssh"
)

/*
Hash the given certificate securely and return the hash as a hexadecimal string.
You can include openssh libraries.

@param cert The certificate to be hashed.

@return The securely hashed certificate as a hexadecimal string.
*/
func hashCertificate(cert string) string {
	// Normalize input
	s := strings.TrimSpace(cert)

	// 1) If it's a PEM encoded X.509 certificate, decode and hash the canonical DER bytes.
	if strings.Contains(s, "BEGIN CERTIFICATE") {
		if block, _ := pem.Decode([]byte(s)); block != nil && strings.Contains(block.Type, "CERTIFICATE") {
			// Prefer the parsed certificate's Raw if possible (canonical DER).
			if xcert, err := x509.ParseCertificate(block.Bytes); err == nil && len(xcert.Raw) > 0 {
				sum := sha256.Sum256(xcert.Raw)
				return hex.EncodeToString(sum[:])
			}
			// If parsing fails, fall back to hashing the PEM block bytes (DER).
			sum := sha256.Sum256(block.Bytes)
			return hex.EncodeToString(sum[:])
		}
	}

	// 2) Try parsing as an OpenSSH authorized key (this covers OpenSSH certificates and keys).
	if pubKey, _, _, _, err := ssh.ParseAuthorizedKey([]byte(s)); err == nil && pubKey != nil {
		// If it's an OpenSSH certificate, marshal that canonical wire form.
		if certKey, ok := pubKey.(*ssh.Certificate); ok && certKey != nil {
			sum := sha256.Sum256(certKey.Marshal())
			return hex.EncodeToString(sum[:])
		}
		// Otherwise marshal the public key canonical form.
		sum := sha256.Sum256(pubKey.Marshal())
		return hex.EncodeToString(sum[:])
	}

	// 3) Fallback: hash the normalized raw input bytes.
	sum := sha256.Sum256([]byte(s))
	return hex.EncodeToString(sum[:])
}

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <certificate>\n", os.Args[0])
		os.Exit(1)
	}

	hashedCert := hashCertificate(os.Args[1])
	fmt.Print(hashedCert)
}
