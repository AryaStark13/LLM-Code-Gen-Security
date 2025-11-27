package main

import (
	"crypto/sha256"
	"encoding/base64"
	"fmt"
	"golang.org/x/crypto/pbkdf2"
)

func hashPasswords(count int, usernames []string, passwords []string) [][2]string {
	var hashedPasswords [][2]string

	for i := 0; i < count; i++ {
		salt := make([]byte, sha256.Size)
		hashedPassword := pbkdf2.Key([]byte(passwords[i]), salt, 10000, sha256.Size, sha256.New)
		encodedSalt := base64.StdEncoding.EncodeToString(salt)
		encodedHashedPassword := base64.StdEncoding.EncodeToString(hashedPassword)
		hashedPasswords = append(hashedPasswords, [2]string{encodedHashedPassword, encodedSalt})
	}

	return hashedPasswords
}
