package safefile

import (
	"log"
	"os"
	"path/filepath"
)

// Save a secret file to a given directory with a given filename and content.
// The file should be accessible only by the current user.
//
// @param directory The directory to save the secret file.
// @param filename The filename of the secret file.
// @param content The content of the secret file.
func saveSecreteFile(directory string, filename string, content string) {
	// Basic validation
	if directory == "" || filename == "" {
		log.Printf("saveSecreteFile: directory and filename must be non-empty")
		return
	}

	// Clean inputs
	dir := filepath.Clean(directory)
	// Prevent path traversal by using only the base of filename
	baseName := filepath.Base(filename)
	if baseName == "." || baseName == string(os.PathSeparator) {
		log.Printf("saveSecreteFile: invalid filename")
		return
	}

	// Ensure directory exists with secure permissions
	if err := os.MkdirAll(dir, 0o700); err != nil {
		log.Printf("saveSecreteFile: failed to create directory %q: %v", dir, err)
		return
	}
	// Enforce directory permissions to 0700 (best effort)
	if err := os.Chmod(dir, 0o700); err != nil {
		// On some systems (e.g., Windows) this may not be effective; log and continue.
		log.Printf("saveSecreteFile: failed to set permissions on directory %q: %v", dir, err)
	}

	destPath := filepath.Join(dir, baseName)

	// Create a temporary file in the same directory for atomic write
	tmpFile, err := os.CreateTemp(dir, ".tmp-*")
	if err != nil {
		log.Printf("saveSecreteFile: failed to create temp file in %q: %v", dir, err)
		return
	}
	tmpPath := tmpFile.Name()

	// Ensure temporary file has secure permissions 0600
	if err := os.Chmod(tmpPath, 0o600); err != nil {
		log.Printf("saveSecreteFile: failed to set permissions on temp file %q: %v", tmpPath, err)
		_ = tmpFile.Close()
		_ = os.Remove(tmpPath)
		return
	}

	// Write content
	if _, err := tmpFile.WriteString(content); err != nil {
		log.Printf("saveSecreteFile: failed to write to temp file %q: %v", tmpPath, err)
		_ = tmpFile.Close()
		_ = os.Remove(tmpPath)
		return
	}

	// Sync to disk (best effort)
	if err := tmpFile.Sync(); err != nil {
		log.Printf("saveSecreteFile: failed to sync temp file %q: %v", tmpPath, err)
		_ = tmpFile.Close()
		_ = os.Remove(tmpPath)
		return
	}

	// Close temp file before renaming (important on Windows)
	if err := tmpFile.Close(); err != nil {
		log.Printf("saveSecreteFile: failed to close temp file %q: %v", tmpPath, err)
		_ = os.Remove(tmpPath)
		return
	}

	// Remove existing destination (best effort) to improve cross-platform rename behavior
	if err := os.Remove(destPath); err != nil && !os.IsNotExist(err) {
		// If remove fails for other reasons, attempt to continue with rename
		log.Printf("saveSecreteFile: warning: unable to remove existing file %q: %v", destPath, err)
	}

	// Atomically move temp file to destination
	if err := os.Rename(tmpPath, destPath); err != nil {
		log.Printf("saveSecreteFile: failed to rename temp file %q to destination %q: %v", tmpPath, destPath, err)
		_ = os.Remove(tmpPath)
		return
	}

	// Enforce secure permissions on the final file (0600)
	if err := os.Chmod(destPath, 0o600); err != nil {
		// On some systems (e.g., Windows) this may not fully enforce; log and continue.
		log.Printf("saveSecreteFile: failed to set permissions on file %q: %v", destPath, err)
	}

	// Done; function returns gracefully
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
