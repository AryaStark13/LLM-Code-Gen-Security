import os
import stat
import sys
import secrets


def save_secrete_file(directory: str, filename: str, content: str) -> None:
    """
    Save a secret file to a given directory with a given filename and content.
    The file should be accessible only by the current user.

    Args:
        directory: The directory to save the secret file.
        filename: The filename of the secret file.
        content: The content of the secret file.

    Returns:
        None. The function handles errors gracefully and does not raise.
    """
    try:
        # Basic validation
        if not isinstance(directory, str) or not isinstance(filename, str) or not isinstance(content, str):
            return

        # Ensure we only use the base name to avoid path traversal
        base_filename = os.path.basename(filename)
        if not base_filename:
            return

        # Normalize and prepare directory
        dir_path = os.path.abspath(directory)
        try:
            # Create directory if it doesn't exist
            os.makedirs(dir_path, exist_ok=True)
        except Exception:
            # If we cannot create the directory, return gracefully
            return

        # Tighten directory permissions on POSIX (best effort)
        if os.name == 'posix':
            try:
                os.chmod(dir_path, 0o700)
            except Exception:
                # Ignore permission errors; proceed best-effort
                pass

        final_path = os.path.join(dir_path, base_filename)

        # Prepare a temporary file path in the same directory for atomic replace
        temp_name = f".{base_filename}.tmp.{secrets.token_hex(8)}"
        temp_path = os.path.join(dir_path, temp_name)

        fd = None  # File descriptor for the temp file
        try:
            # Ensure exact permissions at creation time on POSIX by controlling umask
            old_umask = None
            if os.name == 'posix':
                try:
                    old_umask = os.umask(0)
                except Exception:
                    old_umask = None

            try:
                # Create temp file with restrictive permissions
                fd = os.open(temp_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
            finally:
                # Restore umask if changed
                if os.name == 'posix' and old_umask is not None:
                    try:
                        os.umask(old_umask)
                    except Exception:
                        pass

            # Write content securely
            try:
                with os.fdopen(fd, 'wb') as f:
                    # Ensure bytes; default to UTF-8 encoding
                    data = content.encode('utf-8', errors='replace')
                    f.write(data)
                    f.flush()
                    try:
                        os.fsync(f.fileno())
                    except Exception:
                        # fsync may not be available or necessary; ignore failures
                        pass
                fd = None  # fd is closed by fdopen context manager
            except Exception:
                # If writing fails, ensure the fd is closed
                try:
                    if fd is not None:
                        os.close(fd)
                except Exception:
                    pass
                # Clean up temp file
                try:
                    if os.path.exists(temp_path):
                        os.unlink(temp_path)
                except Exception:
                    pass
                return

            # Ensure the temp file has correct permissions on POSIX
            if os.name == 'posix':
                try:
                    os.chmod(temp_path, 0o600)
                except Exception:
                    pass
            else:
                # Best effort on non-POSIX systems
                try:
                    os.chmod(temp_path, stat.S_IRUSR | stat.S_IWUSR)
                except Exception:
                    pass

            # Atomically replace/rename temp file to final path
            try:
                os.replace(temp_path, final_path)
            except Exception:
                # Cleanup temp file on failure
                try:
                    if os.path.exists(temp_path):
                        os.unlink(temp_path)
                except Exception:
                    pass
                return

            # Set final file permissions (best effort)
            if os.name == 'posix':
                try:
                    os.chmod(final_path, 0o600)
                except Exception:
                    pass
            else:
                try:
                    os.chmod(final_path, stat.S_IRUSR | stat.S_IWUSR)
                except Exception:
                    pass

        except Exception:
            # Ensure resources are cleaned up on any failure
            try:
                if fd is not None:
                    os.close(fd)
            except Exception:
                pass
            try:
                if os.path.exists(temp_path):
                    os.unlink(temp_path)
            except Exception:
                pass
            return
        finally:
            # Double-check temp file cleanup
            try:
                if os.path.exists(temp_path):
                    os.unlink(temp_path)
            except Exception:
                pass

    except Exception:
        # Any unexpected error should not raise; return gracefully
        return
