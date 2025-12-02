package main

import (
	"crypto/rand"
	"crypto/sha256"
	"encoding/base64"
	"golang.org/x/crypto/pbkdf2"
)

func hashPasswords(count int, usernames []string, passwords []string) [][2]string {
	hashedPasswords := make([][2]string, count)
	for i := 0; i < count; i++ {
		salt := make([]byte, 32) // 32 bytes for the salt
		_, err := rand.Read(salt)
		if err != nil {
			// Handle error
			continue
		}
		hashedPassword := pbkdf2.Key([]byte(passwords[i]), salt, 10000, 64, sha256.New)
		hashedPasswords[i] = [2]string{base64.StdEncoding.EncodeToString(hashedPassword), base64.StdEncoding.EncodeToString(salt)}
	}
	return hashedPasswords
}
