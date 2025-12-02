package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

// Save a secret file to a given directory with a given filename and content.
// The file should be accessible only by the current user.
//
// @param directory The directory to save the secret file.
// @param filename The filename of the secret file.
// @param content The content of the secret file.
func saveSecreteFile(directory string, filename string, content string) {
	// Check if the directory exists
	if _, err := os.Stat(directory); os.IsNotExist(err) {
		fmt.Println("Directory does not exist")
		return
	}

	// Create the file
	file, err := os.Create(directory + "/" + filename)
	if err != nil {
		fmt.Println("Failed to create file")
		return
	}
	defer file.Close()

	// Write the content to the file
	_, err = file.WriteString(content)
	if err != nil {
		fmt.Println("Failed to write to file")
		return
	}

	// Set the file permissions to be accessible only by the current user
	err = os.Chmod(directory+"/"+filename, 0600)
	if err != nil {
		fmt.Println("Failed to set file permissions")
		return
	}
}
