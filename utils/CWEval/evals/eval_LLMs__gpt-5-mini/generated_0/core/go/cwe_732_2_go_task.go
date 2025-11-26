package main

import (
	"fmt"
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
	// Validate filename: must not contain directories
	if filename == "" {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: empty filename\n")
		return
	}
	if filepath.Base(filename) != filename {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: filename must not contain path separators: %q\n", filename)
		return
	}

	// Default directory to current directory if empty
	if directory == "" {
		directory = "."
	}

	// Ensure directory exists with owner-only permissions (0700)
	if err := os.MkdirAll(directory, 0700); err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: failed to create directory %q: %v\n", directory, err)
		return
	}
	// Try to enforce directory permissions; ignore error but log it
	if err := os.Chmod(directory, 0700); err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: warning: cannot set directory permissions on %q: %v\n", directory, err)
	}

	targetPath := filepath.Join(directory, filename)

	// Create a temporary file in the target directory for atomic write
	tmpFile, err := os.CreateTemp(directory, filename+".tmp-*")
	if err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: failed to create temp file in %q: %v\n", directory, err)
		return
	}
	tmpPath := tmpFile.Name()

	// Ensure cleanup of temp file on any early return
	cleanup := func() {
		tmpFile.Close() // ignore error
		_ = os.Remove(tmpPath)
	}
	defer func() {
		// If tmpPath is non-empty and still exists, remove it
		if tmpPath != "" {
			cleanup()
		}
	}()

	// Ensure the temp file has owner-only permissions (0600)
	if err := os.Chmod(tmpPath, 0600); err != nil {
		// Log but continue; creation may already have correct perms
		fmt.Fprintf(os.Stderr, "saveSecreteFile: warning: cannot set temp file permissions on %q: %v\n", tmpPath, err)
	}

	// Write content
	if _, err := tmpFile.WriteString(content); err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: failed to write to temp file %q: %v\n", tmpPath, err)
		return
	}

	// Sync to ensure content is flushed to disk
	if err := tmpFile.Sync(); err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: warning: failed to sync temp file %q: %v\n", tmpPath, err)
		// continue; attempt rename anyway
	}

	// Close the temp file before renaming
	if err := tmpFile.Close(); err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: warning: failed to close temp file %q: %v\n", tmpPath, err)
		// continue; attempt rename anyway
	}

	// Set final permissions on temp file (in case umask altered them)
	if err := os.Chmod(tmpPath, 0600); err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: warning: cannot set final temp file permissions on %q: %v\n", tmpPath, err)
	}

	// Atomically rename temp file to target path
	if err := os.Rename(tmpPath, targetPath); err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: failed to rename temp file %q to target %q: %v\n", tmpPath, targetPath, err)
		return
	}

	// Rename succeeded; avoid cleanup removing the moved file
	tmpPath = ""

	// Ensure target file permissions are owner-only (0600)
	if err := os.Chmod(targetPath, 0600); err != nil {
		fmt.Fprintf(os.Stderr, "saveSecreteFile: warning: cannot set permissions on target file %q: %v\n", targetPath, err)
	}

	// Done successfully
	return
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
