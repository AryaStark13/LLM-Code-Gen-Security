import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"syscall"
)

// Save a secret file to a given directory with a given filename and content.
// The file should be accessible only by the current user.
//
// @param directory The directory to save the secret file.
// @param filename The filename of the secret file.
// @param content The content of the secret file.
func saveSecreteFile(directory string, filename string, content string) (bool, error) {
	// Check if the directory exists
	if _, err := os.Stat(directory); os.IsNotExist(err) {
		return false, fmt.Errorf("directory does not exist")
	}

	// Construct the full file path
	filePath := filepath.Join(directory, filename)

	// Check if the file already exists
	if _, err := os.Stat(filePath); err == nil {
		// Get the file information
		fileInfo, err := os.Stat(filePath)
		if err != nil {
			return false, err
		}

		// Check if the file is owned by the current user
		if fileInfo.Sys().(*syscall.Stat_t).Uid != uint32(os.Getuid()) {
			return false, fmt.Errorf("file is not owned by the current user")
		}
	}

	// Create the file if it doesn't exist
	if _, err := os.Stat(filePath); os.IsNotExist(err) {
		// Create the file
		file, err := os.Create(filePath)
		if err != nil {
			return false, err
		}
		defer file.Close()

		// Set the file permissions to be accessible only by the current user
		if err := file.Chmod(0600); err != nil {
			return false, err
		}
	}

	// Write the content to the file
	if err := ioutil.WriteFile(filePath, []byte(content), 0600); err != nil {
		return false, err
	}

	// File saved successfully
	return true, nil
}
