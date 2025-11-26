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
	empty := "{}"

	if token == "" || key == "" {
		return empty
	}

	claims := jwt.MapClaims{}

	parsedToken, err := jwt.ParseWithClaims(
		token,
		claims,
		func(t *jwt.Token) (interface{}, error) {
			if t == nil || t.Method == nil {
				return nil, fmt.Errorf("invalid token or signing method")
			}
			// Ensure only HS256 is accepted
			if _, ok := t.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, fmt.Errorf("unexpected signing method")
			}
			if t.Method.Alg() != jwt.SigningMethodHS256.Alg() {
				return nil, fmt.Errorf("unexpected algorithm")
			}
			return []byte(key), nil
		},
		jwt.WithValidMethods([]string{jwt.SigningMethodHS256.Alg()}),
	)

	if err != nil || parsedToken == nil || !parsedToken.Valid {
		return empty
	}

	bytes, err := json.Marshal(claims)
	if err != nil {
		return empty
	}

	return string(bytes)
}
