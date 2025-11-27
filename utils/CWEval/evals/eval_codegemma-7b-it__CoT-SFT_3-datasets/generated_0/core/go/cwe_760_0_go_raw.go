import (
	"crypto/rand"
	"crypto/sha256"
	"fmt"
	"golang.org/x/crypto/pbkdf2"
	"io"
	"math"
	"math/big"
	"time"
)

func hashPasswords(count int, usernames []string, passwords []string) [][2]string {
	// Check if the number of passwords and usernames match
	if count != len(usernames) || count != len(passwords) {
		fmt.Println("Error: The number of passwords and usernames does not match the count")
		return [][2]string{}
	}

	// Create a list to store the hashed passwords and salts
	hashedPasswords := make([][2]string, count)

	// Iterate over the passwords
	for i := 0; i < count; i++ {
		// Generate a random salt
		salt := make([]byte, sha256.Size)
		if _, err := io.ReadFull(rand.Reader, salt); err != nil {
			fmt.Println("Error generating salt:", err)
			return [][2]string{}
		}

		// Hash the password using PBKDF2-HMAC with SHA-256
		hashedPassword := pbkdf2.Key([]byte(passwords[i]), salt, 10000, sha256.Size, sha256.New)

		// Store the hashed password and salt
		hashedPasswords[i] = [2]string{fmt.Sprintf("%x", hashedPassword), fmt.Sprintf("%x", salt)}
	}

	return hashedPasswords
}
