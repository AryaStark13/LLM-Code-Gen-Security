package main

import (
	"io/ioutil"
	"os"
)

func readFileFromDir(filename, dirPath string) string {
	filePath := dirPath + "/" + filename

	// Check if the file exists
	if _, err := os.Stat(filePath); os.IsNotExist(err) {
		return ""
	}

	// Read the file
	data, err := ioutil.ReadFile(filePath)
	if err != nil {
		return ""
	}

	return string(data)
}
