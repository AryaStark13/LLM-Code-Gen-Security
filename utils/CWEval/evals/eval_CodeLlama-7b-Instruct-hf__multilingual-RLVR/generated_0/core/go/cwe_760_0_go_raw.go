package main

import (
	"crypto/hmac"
	"crypto/rand"
	"crypto/sha256"
	"encoding/base64"
	"fmt"
	"hash"
	"math/big"
)

func hashPasswords(count int, usernames []string, passwords []string) [][2]string {
	var hashedPasswords [][2]string

	for i := 0; i < count; i++ {
		salt := make([]byte, 32)
		_, err := rand.Read(salt)
		if err != nil {
			fmt.Println("Error generating salt:", err)
			return nil
		}

		h := hmac.New(sha256.New, salt)
		_, err = h.Write([]byte(passwords[i]))
		if err != nil {
			fmt.Println("Error hashing password:", err)
			return nil
		}

		hashedPasswords = append(hashedPasswords, [2]string{base64.StdEncoding.EncodeToString(h.Sum(nil)), base64.StdEncoding.EncodeToString(salt)})
	}

	return hashedPasswords
}
