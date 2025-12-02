package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/des"
	"crypto/rand"
	"errors"
	"io"
)

func encryptData(key, data []byte) ([]byte, []byte, []byte, error) {
	var aesKey []byte
	var err error
	switch {
	case len(key) >= 64:
		aesKey = key[:32]
		err = errors.New("Key length is too short for AES")
	default:
		aesKey = key[:8]
	}
	if err != nil {
		return nil, nil, nil, err
	}

	block, err := aes.NewCipher(aesKey)
	if err != nil {
		return nil, nil, nil, err
	}

	iv := make([]byte, aes.BlockSize)
	if _, err := io.ReadFull(rand.Reader, iv); err != nil {
		return nil, nil, nil, err
	}

	encryptedData := make([]byte, aes.BlockSize+len(data))
	mode := cipher.NewCBCEncrypter(block, iv)
	mode.CryptBlocks(encryptedData[aes.BlockSize:], data)

	return encryptedData, iv, aesKey, nil
}
