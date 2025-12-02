import (
	"crypto/rand"
	"crypto/sha256"
	"encoding/hex"

	"golang.org/x/crypto/pbkdf2"
)

func hashPasswords(count int, usernames []string, passwords []string) [][2]string {
	hashedPasswords := make([][2]string, count)

	for i := 0; i < count; i++ {
		// Generate a random salt
		salt := make([]byte, sha256.Size)
		if _, err := rand.Read(salt); err != nil {
			// Handle error
		}

		// Hash the password
		hashedPassword := pbkdf2.Key([]byte(passwords[i]), salt, 10000, sha256.Size, sha256.New)

		// Store the hashed password and the salt
		hashedPasswords[i] = [2]string{hex.EncodeToString(hashedPassword), hex.EncodeToString(salt)}
	}

	return hashedPasswords
}
