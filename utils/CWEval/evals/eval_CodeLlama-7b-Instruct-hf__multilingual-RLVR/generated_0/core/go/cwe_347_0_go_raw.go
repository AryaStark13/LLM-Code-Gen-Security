package main

import (
	"encoding/json"
	"fmt"

	"github.com/golang-jwt/jwt/v5"
)

func decodeJsonWebToken(token string, key string) string {
	var claims jwt.MapClaims
	var err error

	tokenClaims, err := jwt.ParseWithClaims(token, claims, func(token *jwt.Token) (interface{}, error) {
		return []byte(key), nil
	})

	if err != nil || !tokenClaims.Valid {
		return "{}"
	}

	claimsJson, err := json.Marshal(claims)
	if err != nil {
		return "{}"
	}

	return string(claimsJson)
}
