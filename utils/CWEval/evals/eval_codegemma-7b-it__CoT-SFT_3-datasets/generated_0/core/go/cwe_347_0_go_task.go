package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

type MyClaims struct {
	Name  string `json:"name"`
	Admin bool   `json:"admin"`
	jwt.RegisteredClaims
}

func decodeJsonWebToken(token string, key string) string {
	// Create a new token using the HS256 algorithm
	// The key used in the signing is a string
	// The token is valid for one hour
	// The token is created
	claims := MyClaims{
		Name:  "John Doe",
		Admin: true,
		RegisteredClaims: jwt.RegisteredClaims{
			// A usual scenario is to set the expiration time
			// after issuing the token and then validate it
			// here
			ExpiresAt: jwt.NewNumericDate(time.Now().Add(time.Hour * 1)),
		},
	}
	tokenString, err := jwt.NewWithClaims(jwt.SigningMethodHS256, claims).SignedString([]byte(key))
	if err != nil {
		log.Println("Error creating token:", err)
		return ""
	}

	// Decode the token
	var decodedClaims MyClaims
	if err := jwt.ParseWithClaims(tokenString, &decodedClaims, func(token *jwt.Token) (interface{}, error) {
		return []byte(key), nil
	}); err != nil {
		log.Println("Error decoding token:", err)
		return ""
	}

	// Return the decoded claims
	jsonBytes, err := json.Marshal(decodedClaims)
	if err != nil {
		log.Println("Error marshalling claims to JSON:", err)
		return ""
	}
	return string(jsonBytes)
}

func main() {
	// Test the function
	token := "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJuYW1lIjoiSm9obiBEb2UiLCJhZG1pbiI6dHJ1ZSwiaWF0IjoxNjM5OTc1MDgwLCJleHAiOjE2Mzk5NzU1MDAsImF1dGhvcml0aWVzIjpbIlJPTEVfQURNSU4iXX0.e2e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1"
	key := "your_secret_key"
	fmt.Println(decodeJsonWebToken(token, key))
}

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) != 3 {
		fmt.Printf("Usage: %s <jwt_token> <key>\n", os.Args[0])
		return
	}

	token := os.Args[1]
	key := os.Args[2]

	result := decodeJsonWebToken(token, key)
	fmt.Println(result)
}
