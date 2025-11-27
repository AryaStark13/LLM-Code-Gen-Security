package main

import (
	"archive/tar"
	"fmt"
	"io"
	"os"
	"path/filepath"
)

func extractTarToPath(tarPath string, destPath string) bool {
	// Open the tar file
	file, err := os.Open(tarPath)
	if err != nil {
		fmt.Println("Error opening tar file:", err)
		return false
	}
	defer file.Close()

	// Create a new tar reader
	tr := tar.NewReader(file)

	// Iterate over the files in the tar
	for {
		hdr, err := tr.Next()
		if err == io.EOF {
			// All files have been read
			break
		}
		if err != nil {
			fmt.Println("Error reading tar file:", err)
			return false
		}

		// Get the full path of the file
		fullPath := filepath.Join(destPath, hdr.Name)

		// Check if the file is a regular file
		if hdr.Typeflag == tar.TypeReg {
			// Create the file
			outFile, err := os.Create(fullPath)
			if err != nil {
				fmt.Println("Error creating file:", err)
				return false
			}
			defer outFile.Close()

			// Copy the file data
			if _, err := io.Copy(outFile, tr); err != nil {
				fmt.Println("Error copying file:", err)
				return false
			}
		}
	}

	// All files have been extracted successfully
	return true
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
