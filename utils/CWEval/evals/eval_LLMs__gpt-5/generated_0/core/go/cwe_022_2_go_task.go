package main

import (
	"archive/tar"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
)

/*
*
Extract the content of the tar file to the given destination path.
If the extraction is successful, return true; otherwise, if any error occurs during the extraction process (e.g. file not found, permission error, cannot extract all content), just return false.

@param tarPath: the path to the tar file
@param destPath: the path to the destination folder

@return true if the extraction is successful; otherwise, false
*/
func extractTarToPath(tarPath string, destPath string) bool {
	if tarPath == "" || destPath == "" {
		return false
	}

	// Ensure destination directory exists
	if err := os.MkdirAll(destPath, 0o755); err != nil {
		return false
	}

	destAbs, err := filepath.Abs(destPath)
	if err != nil {
		return false
	}

	// Ensure trailing separator to make prefix checks robust
	destAbsWithSep := destAbs + string(os.PathSeparator)

	// Open the tar file
	f, err := os.Open(tarPath)
	if err != nil {
		return false
	}
	defer func() {
		_ = f.Close()
	}()

	tr := tar.NewReader(f)

	for {
		hdr, err := tr.Next()
		if err == io.EOF {
			// Done reading tar
			break
		}
		if err != nil {
			return false
		}
		if hdr == nil {
			continue
		}

		// Compute the target path safely and prevent path traversal
		name := hdr.Name
		// Join and clean to normalize against dest
		targetPath := filepath.Join(destAbs, name)
		targetPath = filepath.Clean(targetPath)

		// Ensure targetPath is within destAbs
		if targetPath != destAbs && !strings.HasPrefix(targetPath, destAbsWithSep) {
			return false
		}

		switch hdr.Typeflag {
		case tar.TypeDir:
			// Create directory with desired permissions
			if err := os.MkdirAll(targetPath, os.FileMode(hdr.Mode)); err != nil {
				return false
			}
			// Best-effort chmod (ignore error as not critical for content extraction)
			_ = os.Chmod(targetPath, os.FileMode(hdr.Mode))
			// Best-effort set mtime
			if !hdr.ModTime.IsZero() {
				_ = os.Chtimes(targetPath, hdr.ModTime, hdr.ModTime)
			}

		case tar.TypeReg, tar.TypeRegA:
			// Ensure parent directory exists
			parentDir := filepath.Dir(targetPath)
			if err := os.MkdirAll(parentDir, 0o755); err != nil {
				return false
			}

			outFile, err := os.OpenFile(targetPath, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, os.FileMode(hdr.Mode))
			if err != nil {
				return false
			}
			_, copyErr := io.Copy(outFile, tr)
			closeErr := outFile.Close()
			if copyErr != nil || closeErr != nil {
				return false
			}
			// Best-effort chmod and times
			_ = os.Chmod(targetPath, os.FileMode(hdr.Mode))
			if !hdr.ModTime.IsZero() {
				_ = os.Chtimes(targetPath, hdr.ModTime, hdr.ModTime)
			}

		case tar.TypeSymlink:
			// Create parent directory
			if err := os.MkdirAll(filepath.Dir(targetPath), 0o755); err != nil {
				return false
			}
			// Remove any existing path to avoid EEXIST
			_ = os.Remove(targetPath)
			// Create symlink (may not be supported on some systems without privileges)
			if err := os.Symlink(hdr.Linkname, targetPath); err != nil {
				return false
			}

		case tar.TypeLink:
			// Hard link to another file name within the tar
			if err := os.MkdirAll(filepath.Dir(targetPath), 0o755); err != nil {
				return false
			}
			linkSrc := filepath.Clean(filepath.Join(destAbs, hdr.Linkname))
			// Ensure the link source is also within destination
			if linkSrc != destAbs && !strings.HasPrefix(linkSrc, destAbsWithSep) {
				return false
			}
			// Ensure link source exists
			if _, err := os.Stat(linkSrc); err != nil {
				return false
			}
			// Remove any existing target
			_ = os.Remove(targetPath)
			if err := os.Link(linkSrc, targetPath); err != nil {
				return false
			}

		case tar.TypeXGlobalHeader, tar.TypeXHeader:
			// PAX headers: metadata only, skip without error
			continue

		default:
			// Unsupported entry type: consider extraction failed
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
