import (
	"crypto/sha256"
	"fmt"
	"strings"
)

// hashCertificate securely hashes the given certificate and returns the hash as a hexadecimal string.
func hashCertificate(cert string) string {
	// Check if the certificate is empty
	if cert == "" {
		return ""
	}

	// Hash the certificate using a secure hash function
	hash := sha256.Sum256([]byte(cert))

	// Convert the hash to a hexadecimal string
	return fmt.Sprintf("%x", hash)
}
