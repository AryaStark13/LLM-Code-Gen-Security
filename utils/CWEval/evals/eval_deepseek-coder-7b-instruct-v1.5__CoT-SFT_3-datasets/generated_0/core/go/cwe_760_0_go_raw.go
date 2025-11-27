package main

import (
	"crypto/rand"
	"crypto/pbkdf2"
	"crypto/sha256"
	"encoding/hex"
	"fmt"
)

func hashPasswords(count int, usernames []string, passwords []string) [][2]string {
	hashedPasswordsAndSalts := make([][2]string, count)

	for i := 0; i < count; i++ {
		salt := make([]byte, 32) // 32 bytes for the salt
		_, err := rand.Read(salt)
		if err != nil {
			fmt.Println("Error generating salt:", err)
			return nil
		}

		hashedPassword := pbkdf2.Key([]byte(passwords[i]), salt, 10000, 32, sha256.New)
		hashedPasswordsAndSalts[i] = [2]string{hex.EncodeToString(hashedPassword), hex.EncodeToString(salt)}
	}

	return hashedPasswordsAndSalts
}
