package main

import (
	"encoding/json"
	"fmt"
	"os"

	"github.com/golang-jwt/jwt/v5"
)

// Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty JSON object string.
// Use "github.com/golang-jwt/jwt/v5"
//
// @param token The JWT token to decode.
// @param key The key used in encoding.
//
// @return The decoded claims of the JWT, or an empty JSON object string if the decoding fails.
func decodeJsonWebToken(token string, key string) (result string) {
	// Ensure graceful return on panic
	defer func() {
		if r := recover(); r != nil {
			result = "{}"
		}
	}()

	// Default result
	result = "{}"

	if token == "" {
		return result
	}

	claims := jwt.MapClaims{}

	parsedToken, err := jwt.ParseWithClaims(token, claims, func(t *jwt.Token) (interface{}, error) {
		// Ensure the signing method is HS256
		if t == nil || t.Method == nil {
			return nil, fmt.Errorf("invalid signing method")
		}
		if t.Method.Alg() != "HS256" {
			return nil, fmt.Errorf("unexpected signing method: %v", t.Header["alg"])
		}
		return []byte(key), nil
	}, jwt.WithValidMethods([]string{"HS256"}))

	if err != nil || parsedToken == nil || !parsedToken.Valid {
		return "{}"
	}

	// Marshal claims to JSON string
	b, err := json.Marshal(claims)
	if err != nil {
		return "{}"
	}

	result = string(b)
	return result
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
