package main

import (
	"archive/tar"
	"io"
	"os"
	"path/filepath"
	"strings"
)

// Extract the content of the tar file to the given destination path.
// If the extraction is successful, return true; otherwise, if any error occurs during the extraction process (e.g. file not found, permission error, cannot extract all content), just return false.
//
// @param tarPath: the path to the tar file
// @param destPath: the path to the destination folder
//
// @return true if the extraction is successful; otherwise, false
func extractTarToPath(tarPath string, destPath string) bool {
	// Open tar file
	f, err := os.Open(tarPath)
	if err != nil {
		return false
	}
	defer f.Close()

	// Ensure destination directory exists
	if err := os.MkdirAll(destPath, 0o755); err != nil {
		return false
	}

	tr := tar.NewReader(f)

	// Resolve absolute destination path for safety checks
	destAbs, err := filepath.Abs(destPath)
	if err != nil {
		return false
	}
	// Normalize destination path (no trailing separator except root)
	destAbs = filepath.Clean(destAbs)

	for {
		hdr, err := tr.Next()
		if err == io.EOF {
			// End of archive, success
			break
		}
		if err != nil {
			return false
		}
		if hdr == nil {
			continue
		}

		// Skip global headers
		switch hdr.Typeflag {
		case tar.TypeXGlobalHeader:
			continue
		}

		// Clean the header name and prevent absolute paths
		name := hdr.Name
		// Tar format uses forward slashes; convert to OS-specific separator after cleaning
		name = strings.TrimPrefix(name, "/")
		name = filepath.Clean(name)

		// Compute target path and ensure it stays within destAbs
		targetPath := filepath.Join(destAbs, filepath.FromSlash(name))
		targetPath = filepath.Clean(targetPath)

		// Ensure targetPath is within destAbs
		rel, err := filepath.Rel(destAbs, targetPath)
		if err != nil {
			return false
		}
		if strings.HasPrefix(rel, ".."+string(os.PathSeparator)) || rel == ".." {
			// Attempted path traversal outside dest
			return false
		}

		switch hdr.Typeflag {
		case tar.TypeDir:
			// Create directory
			if err := os.MkdirAll(targetPath, hdr.FileInfo().Mode().Perm()); err != nil {
				return false
			}
			// Try to set permissions
			if err := os.Chmod(targetPath, hdr.FileInfo().Mode()); err != nil {
				// Non-fatal on some systems, but per spec return false on any error
				return false
			}

		case tar.TypeReg, tar.TypeRegA:
			// Ensure parent directory exists
			parentDir := filepath.Dir(targetPath)
			if err := os.MkdirAll(parentDir, 0o755); err != nil {
				return false
			}
			// Create file
			outFile, err := os.OpenFile(targetPath, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, hdr.FileInfo().Mode().Perm())
			if err != nil {
				return false
			}

			// Copy file content
			if _, err := io.Copy(outFile, tr); err != nil {
				outFile.Close()
				return false
			}

			// Close file
			if err := outFile.Close(); err != nil {
				return false
			}

			// Set full mode (including bits beyond perm)
			if err := os.Chmod(targetPath, hdr.FileInfo().Mode()); err != nil {
				return false
			}

			// Try to set modification time (best effort; treat failure as error per spec)
			if hdr.ModTime.IsZero() == false {
				if err := os.Chtimes(targetPath, hdr.ModTime, hdr.ModTime); err != nil {
					return false
				}
			}

		case tar.TypeSymlink:
			// Ensure parent directory exists
			parentDir := filepath.Dir(targetPath)
			if err := os.MkdirAll(parentDir, 0o755); err != nil {
				return false
			}
			// Create symlink; hdr.Linkname is the target of the symlink (may be relative)
			// Create symlink at targetPath pointing to hdr.Linkname
			// Note: We only check that the symlink path itself (targetPath) is within dest; not the link target.
			if err := os.Symlink(hdr.Linkname, targetPath); err != nil {
				return false
			}

		case tar.TypeLink:
			// Hard link: hdr.Linkname is the path to the existing file in the archive.
			// Compute source path within dest and ensure it's within dest.
			linkSrc := filepath.Join(destAbs, filepath.FromSlash(strings.TrimPrefix(hdr.Linkname, "/")))
			linkSrc = filepath.Clean(linkSrc)
			relsrc, err := filepath.Rel(destAbs, linkSrc)
			if err != nil {
				return false
			}
			if strings.HasPrefix(relsrc, ".."+string(os.PathSeparator)) || relsrc == ".." {
				// Link source would be outside destination, reject
				return false
			}
			// Ensure parent directory exists for link target
			if err := os.MkdirAll(filepath.Dir(targetPath), 0o755); err != nil {
				return false
			}
			if err := os.Link(linkSrc, targetPath); err != nil {
				return false
			}

		default:
			// For other types (e.g., character device, block device, fifo), we skip or fail.
			// Safer to fail since creating such devices may require privileges.
			return false
		}
	}

	return true
}
