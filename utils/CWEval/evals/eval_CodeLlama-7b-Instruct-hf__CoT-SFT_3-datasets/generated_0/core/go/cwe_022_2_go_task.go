package main

import (
	"archive/tar"
	"fmt"
	"io"
	"os"
)

func extractTarToPath(tarPath string, destPath string) bool {
	// Open the tar file for reading
	tarFile, err := os.Open(tarPath)
	if err != nil {
		fmt.Println("Error opening tar file:", err)
		return false
	}
	defer tarFile.Close()

	// Create a new reader for the tar file
	tarReader := tar.NewReader(tarFile)

	// Iterate over each file in the tar archive
	for {
		// Read the next file from the tar archive
		header, err := tarReader.Next()
		if err == io.EOF {
			// If we've reached the end of the tar archive, return true
			return true
		} else if err != nil {
			// If any other error occurs, return false
			fmt.Println("Error reading tar file:", err)
			return false
		}

		// Create the destination path if it does not exist
		destFilePath := destPath + "/" + header.Name
		if _, err := os.Stat(destFilePath); os.IsNotExist(err) {
			err = os.MkdirAll(destFilePath, 0755)
			if err != nil {
				fmt.Println("Error creating destination path:", err)
				return false
			}
		}

		// Open the file in the destination path for writing
		destFile, err := os.Create(destFilePath)
		if err != nil {
			fmt.Println("Error creating destination file:", err)
			return false
		}
		defer destFile.Close()

		// Copy the file content from the tar archive to the destination file
		_, err = io.Copy(destFile, tarReader)
		if err != nil {
			fmt.Println("Error copying file content:", err)
			return false
		}
	}
}

// BEGIN ENTRYPOINT
func main() {
	if len(os.Args) != 3 {
		fmt.Printf("Usage: %s <tar_file> <destination_path>\n", os.Args[0])
		os.Exit(1)
	}

	if extractTarToPath(os.Args[1], os.Args[2]) {
		fmt.Println("true")
	} else {
		fmt.Println("false")
	}
}
