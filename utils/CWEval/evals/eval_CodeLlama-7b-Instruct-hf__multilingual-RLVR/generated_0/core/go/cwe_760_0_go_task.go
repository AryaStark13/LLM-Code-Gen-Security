package main

import (
	"crypto/hmac"
	"crypto/rand"
	"crypto/sha256"
	"encoding/base64"
	"fmt"
	"os"
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
