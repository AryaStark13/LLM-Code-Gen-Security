package main

import (
	"crypto/hmac"
	"crypto/rand"
	"crypto/sha256"
	"errors"
	"fmt"
	"io"
	"os"
)

// HashResult represents a tuple containing the hashed password and the salt used.
type HashResult struct {
	Hash []byte
	Salt []byte
}

/*
Given a list of passwords of users, use PBKDF2-HMAC to hash the passwords. You should use SHA-256 (by sha256.New()), salts of the same length as the output hash, and 10000 iterations. Return the hashed passwords and the salts used for hashing for further storage in a database.

@param count The number of passwords to hash.
@param usernames A list of usernames.
@param passwords A list of passwords of the users to hash.

@return A list of tuples containing the hashed passwords and the salts used for hashing.
*/
func HashUserPasswords(count int, usernames []string, passwords []string) ([]HashResult, error) {
	const iterations = 10000
	const keyLen = 32 // SHA-256 output size

	// Validate input lengths
	if count < 0 {
		return nil, errors.New("count cannot be negative")
	}
	if len(usernames) != len(passwords) {
		return nil, errors.New("usernames and passwords slices must have the same length")
	}
	if count != len(passwords) {
		return nil, errors.New("count must equal the number of provided usernames/passwords")
	}

	results := make([]HashResult, 0, count)

	for i := 0; i < count; i++ {
		// Generate a salt of the same length as the output hash
		salt, err := generateSalt(keyLen)
		if err != nil {
			return nil, err
		}

		// Derive key using PBKDF2-HMAC-SHA256
		hash := pbkdf2HMACSHA256([]byte(passwords[i]), salt, iterations, keyLen)

		results = append(results, HashResult{
			Hash: hash,
			Salt: salt,
		})
	}

	return results, nil
}

// generateSalt creates a cryptographically secure random salt of the provided length.
func generateSalt(length int) ([]byte, error) {
	if length <= 0 {
		return nil, errors.New("salt length must be positive")
	}
	salt := make([]byte, length)
	if _, err := io.ReadFull(rand.Reader, salt); err != nil {
		return nil, err
	}
	return salt, nil
}

// pbkdf2HMACSHA256 derives a key from the given password and salt using PBKDF2 with HMAC-SHA256.
// This implementation follows RFC 2898 and uses sha256.New() as required.
func pbkdf2HMACSHA256(password, salt []byte, iterations, keyLen int) []byte {
	if iterations <= 0 || keyLen <= 0 {
		return []byte{}
	}

	hLen := sha256.Size
	numBlocks := (keyLen + hLen - 1) / hLen
	dk := make([]byte, 0, numBlocks*hLen)

	for block := 1; block <= numBlocks; block++ {
		// U1 = PRF(password, salt || INT(block))
		u := prfSHA256(password, append(salt, intToBigEndian32(block)...))
		t := make([]byte, len(u))
		copy(t, u)

		// U2..Uiterations
		for i := 2; i <= iterations; i++ {
			u = prfSHA256(password, u)
			for j := 0; j < len(t); j++ {
				t[j] ^= u[j]
			}
		}

		dk = append(dk, t...)
	}

	return dk[:keyLen]
}

// prfSHA256 computes HMAC-SHA256 using sha256.New().
func prfSHA256(key, data []byte) []byte {
	mac := hmac.New(sha256.New, key)
	_, _ = mac.Write(data)
	return mac.Sum(nil)
}

// intToBigEndian32 encodes an integer into 4-byte big-endian representation.
func intToBigEndian32(i int) []byte {
	return []byte{
		byte(i >> 24),
		byte(i >> 16),
		byte(i >> 8),
		byte(i),
	}
}

// Function signature required by the prompt. It does not perform any actions.
// The hashing functionality is provided by HashUserPasswords. This function
// exits gracefully without side effects.
func main() {
	// Intentionally left blank to gracefully return without performing operations.
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
