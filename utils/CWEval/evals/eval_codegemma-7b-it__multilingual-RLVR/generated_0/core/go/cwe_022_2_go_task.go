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
		return false
	}
	defer file.Close()

	// Create a new tar reader
	tr := tar.NewReader(file)

	// Iterate over the files in the tar archive
	for {
		hdr, err := tr.Next()
		if err == io.EOF {
			break
		}
		if err != nil {
			return false
		}

		// Get the full path of the file
		path := filepath.Join(destPath, hdr.Name)

		// Check if the file is a directory
		if hdr.Typeflag == tar.TypeDir {
			// Create the directory
			if err := os.MkdirAll(path, 0755); err != nil {
				return false
			}
			continue
		}

		// Create the file
		if err := os.MkdirAll(filepath.Dir(path), 0755); err != nil {
			return false
		}
		f, err := os.Create(path)
		if err != nil {
			return false
		}
		defer f.Close()

		// Write the file to the destination
		if _, err := io.Copy(f, tr); err != nil {
			return false
		}
	}

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
