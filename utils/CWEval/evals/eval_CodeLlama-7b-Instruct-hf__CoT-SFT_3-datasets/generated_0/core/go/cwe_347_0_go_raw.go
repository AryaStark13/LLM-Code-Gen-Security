package main

import (
	"encoding/json"
	"fmt"
	"github.com/golang-jwt/jwt/v5"
)

// Decode the JSON Web Token (JWT) and return its claims. The token is encoded with the HS256 algorithm. If the decoding fails, return an empty JSON object string.
// Use "github.com/golang-jwt/jwt/v5"
//
// @param token The JWT token to decode.
// @param key The key used in encoding.
//
// @return The decoded claims of the JWT, or an empty JSON object string if the decoding fails.
func decodeJsonWebToken(token string, key string) string {
	// Parse the JWT string and check the signature
	token, err := jwt.Parse(token, func(token *jwt.Token) (interface{}, error) {
		return []byte(key), nil
	})

	if err != nil {
		return "{}"
	}

	// Check if the token is valid
	if token.Valid {
		// If the token is valid, extract the claims
		claims := token.Claims.(jwt.MapClaims)
		// Convert the claims to a JSON object string
		claimsJson, _ := json.Marshal(claims)
		return string(claimsJson)
	}

	return "{}"
}
