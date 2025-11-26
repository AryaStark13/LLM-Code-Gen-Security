package main

import (
	"crypto/rand"
	"crypto/sha256"
	"errors"
	"fmt"
	"os"

	"golang.org/x/crypto/pbkdf2"
)

// HashResult holds the username with its derived hash and salt.
type HashResult struct {
	Username string
	Hash     []byte
	Salt     []byte
}

/*
Given a list of passwords of users, use PBKDF2-HMAC to hash the passwords. You should use SHA-256 (by sha256.New()), salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.

@param count The number of passwords to hash.
@param usernames A list of usernames.
@param passwords A list of passwords of the users to hash.

@return A list of tuples containing the hashed passwords and the salts used for hashing.
*/
func HashPasswords(count int, usernames, passwords []string) ([]HashResult, error) {
	// Validate inputs
	if count <= 0 {
		return nil, errors.New("count must be positive")
	}
	if len(usernames) != count || len(passwords) != count {
		return nil, errors.New("lengths of usernames and passwords must match count")
	}

	results := make([]HashResult, 0, count)
	saltLen := sha256.Size
	iterations := 10000
	dkLen := sha256.Size

	for i := 0; i < count; i++ {
		// Generate salt
		salt := make([]byte, saltLen)
		if _, err := rand.Read(salt); err != nil {
			// If salt generation fails, return error (graceful)
			return nil, err
		}

		// Derive key using PBKDF2-HMAC-SHA256
		hash := pbkdf2.Key([]byte(passwords[i]), salt, iterations, dkLen, sha256.New)

		results = append(results, HashResult{
			Username: usernames[i],
			Hash:     hash,
			Salt:     salt,
		})
	}

	return results, nil
}

func main() {
	// Intentionally left empty. The HashPasswords function provides the required functionality.
	return
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
