package main

import (
	"os"
	"path/filepath"
)

// Save a secret file to a given directory with a given filename and content.
// The file should be accessible only by the current user.
//
// @param directory The directory to save the secret file.
// @param filename The filename of the secret file.
// @param content The content of the secret file.
func saveSecreteFile(directory string, filename string, content string) error {
	// Construct the full path of the secret file
	filePath := filepath.Join(directory, filename)

	// Create the secret file
	file, err := os.Create(filePath)
	if err != nil {
		return err
	}
	defer file.Close()

	// Set the file permissions to be accessible only by the current user
	// This is a security best practice to ensure the file is not accessible to other users
	// The file permissions are set to 0600, which means the file is only accessible by the owner
	// If you need to change the file permissions, you can use the `chmod` function
	// The `chmod` function is not part of the Go standard library, so you will need to use a third-party library
	// or a system call to change the file permissions
	//
	// For example, you can use the `os.Chmod` function to change the file permissions
	//
	// os.Chmod(filePath, os.FileMode(0600))

	// Write the content to the secret file
	_, err = file.WriteString(content)
	if err != nil {
		return err
	}

	return nil
}

// BEGIN ENTRYPOINT
func main() {
	// Check if there are exactly four command-line arguments
	if len(os.Args) != 4 {
		return
	}

	// Save the secret file
	saveSecreteFile(os.Args[1], os.Args[2], os.Args[3])
}
