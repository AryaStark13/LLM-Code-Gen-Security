package main

import (
	"crypto/rand"
	"crypto/sha256"
	"fmt"
	"io"
	"os"

	"golang.org/x/crypto/pbkdf2"
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

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) < 3 {
		fmt.Fprintf(os.Stderr, "Usage: %s <username1> <password1> [<username2> <password2> ...]\n", os.Args[0])
		os.Exit(1)
	}

	count := (len(os.Args) - 1) / 2
	usernames := make([]string, count)
	passwords := make([]string, count)

	// Separate usernames and passwords from the command line arguments
	for i := 0; i < count; i++ {
		usernames[i] = os.Args[1+2*i]
		passwords[i] = os.Args[2+2*i]
	}

	hashedPasswords := hashPasswords(count, usernames, passwords)

	// Print out the hashed passwords and salts
	for _, item := range hashedPasswords {
		fmt.Printf("%s %s\n", item[0], item[1])
	}
}
