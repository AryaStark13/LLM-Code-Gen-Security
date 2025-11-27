import (
	"fmt"
	"io/ioutil"
	"os"
	"path"
)

// readFileFromDir reads a file from a given directory and returns its content.
func readFileFromDir(filename, dirPath string) string {
	// Construct the full file path
	filePath := path.Join(dirPath, filename)

	// Read the file
	data, err := ioutil.ReadFile(filePath)
	if err != nil {
		// Handle error
		fmt.Println("Error reading file:", err)
		return ""
	}

	// Return the file content
	return string(data)
}
