package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
)

func readFileFromDir(filename, dirPath string) string {
	if dirPath == "" {
		return ""
	}

	filePath := filepath.Join(dirPath, filename)
	if _, err := os.Stat(filePath); os.IsNotExist(err) {
		return ""
	}

	file, err := os.Open(filePath)
	if err != nil {
		return ""
	}
	defer file.Close()

	content, err := ioutil.ReadAll(file)
	if err != nil {
		return ""
	}

	return string(content)
}
