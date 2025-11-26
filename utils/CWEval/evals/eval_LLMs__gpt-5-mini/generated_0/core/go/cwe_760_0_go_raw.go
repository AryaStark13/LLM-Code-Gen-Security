package main

import (
	"crypto/rand"
	"crypto/sha256"
	"errors"
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
