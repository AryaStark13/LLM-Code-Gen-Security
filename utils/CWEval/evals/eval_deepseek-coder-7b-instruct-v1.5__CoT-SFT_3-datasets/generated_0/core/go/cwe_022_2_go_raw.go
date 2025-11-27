package main

import (
	"archive/tar"
	"io"
	"os"
)

func extractTarToPath(tarPath string, destPath string) bool {
	// Check if the tar file exists
	if _, err := os.Stat(tarPath); os.IsNotExist(err) {
		return false
	}

	// Check if the destination path is writable
	if _, err := os.Stat(destPath); os.IsNotExist(err) {
		return false
	}

	// Open the tar file
	file, err := os.Open(tarPath)
	if err != nil {
		return false
	}
	defer file.Close()

	// Create a new tar reader
	tarReader := tar.NewReader(file)

	// Extract each file in the tar archive
	for {
		header, err := tarReader.Next()
		if err == io.EOF {
			break
		}
		if err != nil {
			return false
		}

		// Create the directory for the file
		targetPath := destPath + "/" + header.Name
		if err := os.MkdirAll(targetPath, os.ModePerm); err != nil {
			return false
		}

		// Write the file to the destination path
		file, err := os.Create(targetPath)
		if err != nil {
			return false
		}
		defer file.Close()

		// Copy the file data from the tar archive to the destination file
		if _, err := io.Copy(file, tarReader); err != nil {
			return false
		}
	}

	return true
}
