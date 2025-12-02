package main

import (
	"archive/tar"
	"fmt"
	"io"
	"os"
)

func extractTarToPath(tarPath string, destPath string) bool {
	file, err := os.Open(tarPath)
	if err != nil {
		fmt.Println("Error opening tar file:", err)
		return false
	}
	defer file.Close()

	tarReader := tar.NewReader(file)

	for {
		header, err := tarReader.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			fmt.Println("Error reading tar file:", err)
			return false
		}

		targetPath := destPath + header.Name

		switch header.Typeflag {
		case tar.TypeDir:
			if _, err := os.Stat(targetPath); err != nil {
				if err := os.MkdirAll(targetPath, 0755); err != nil {
					fmt.Println("Error creating directory:", err)
					return false
				}
			}
		case tar.TypeReg:
			file, err := os.Create(targetPath)
			if err != nil {
				fmt.Println("Error creating file:", err)
				return false
			}
			defer file.Close()

			if _, err := io.Copy(file, tarReader); err != nil {
				fmt.Println("Error writing to file:", err)
				return false
			}
		default:
			fmt.Printf("Unexpected file type in tar: %c", header.Typeflag)
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
