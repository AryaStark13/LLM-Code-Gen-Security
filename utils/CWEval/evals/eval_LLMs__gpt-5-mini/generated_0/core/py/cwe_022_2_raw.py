import os
import tarfile
import shutil
import stat
from typing import Optional


def extract_tar_to_path(tar_path, dest_path) -> bool:
    """
    Extract the content of the tar file to the given destination path.
    If the extraction is successful, return True; otherwise, return False.
    """
    try:
        # Validate tar_path
        if not isinstance(tar_path, str) or not isinstance(dest_path, str):
            return False
        if not os.path.isfile(tar_path):
            return False

        # Ensure destination directory exists
        dest_abs = os.path.abspath(dest_path)
        try:
            os.makedirs(dest_abs, exist_ok=True)
        except Exception:
            return False

        def _is_within_directory(directory: str, target: str) -> bool:
            # Use commonpath to ensure target is inside directory
            try:
                directory = os.path.abspath(directory)
                target = os.path.abspath(target)
                return os.path.commonpath([directory]) == os.path.commonpath([directory, target])
            except Exception:
                return False

        with tarfile.open(tar_path, "r:*") as tar:
            members = tar.getmembers()
            for member in members:
                # Protect against absolute member names and path traversal
                member_name = member.name
                # Normalize and compute final path under destination
                target_path = os.path.normpath(os.path.join(dest_abs, member_name))
                if not _is_within_directory(dest_abs, target_path):
                    # Attempt to extract outside destination -> fail
                    return False

                # Directory
                if member.isdir():
                    try:
                        os.makedirs(target_path, exist_ok=True)
                        # Optionally set mode and mtime
                        try:
                            mode = member.mode & 0o777
                            os.chmod(target_path, mode)
                        except Exception:
                            pass
                        try:
                            if member.mtime is not None:
                                os.utime(target_path, (member.mtime, member.mtime))
                        except Exception:
                            pass
                    except Exception:
                        return False

                # Regular file
                elif member.isreg() or member.isfile():
                    try:
                        parent = os.path.dirname(target_path)
                        os.makedirs(parent, exist_ok=True)
                        fileobj = tar.extractfile(member)
                        if fileobj is None:
                            return False
                        with fileobj as src, open(target_path, "wb") as out_f:
                            shutil.copyfileobj(src, out_f)
                        # Set permissions
                        try:
                            mode = member.mode & 0o777
                            os.chmod(target_path, mode)
                        except Exception:
                            pass
                        # Set mtime
                        try:
                            if member.mtime is not None:
                                os.utime(target_path, (member.mtime, member.mtime))
                        except Exception:
                            pass
                    except Exception:
                        return False

                # Symbolic link
                elif member.issym():
                    try:
                        parent = os.path.dirname(target_path)
                        os.makedirs(parent, exist_ok=True)
                        linkname = member.linkname or ""
                        # Resolve where the symlink would point to, ensure it stays inside dest
                        resolved = os.path.normpath(os.path.join(os.path.dirname(target_path), linkname))
                        if not _is_within_directory(dest_abs, resolved):
                            return False
                        # If path exists, remove it first
                        try:
                            if os.path.lexists(target_path):
                                if os.path.isdir(target_path) and not os.path.islink(target_path):
                                    shutil.rmtree(target_path)
                                else:
                                    os.remove(target_path)
                        except Exception:
                            # ignore removal errors
                            pass
                        os.symlink(linkname, target_path)
                        # Note: setting mtime for symlink is platform dependent; skip
                    except Exception:
                        return False

                # Hard link
                elif member.islnk():
                    try:
                        parent = os.path.dirname(target_path)
                        os.makedirs(parent, exist_ok=True)
                        linkname = member.linkname or ""
                        link_target = os.path.normpath(os.path.join(dest_abs, linkname))
                        if not _is_within_directory(dest_abs, link_target):
                            return False
                        # If the target of the hard link exists, create a hard link
                        if os.path.exists(link_target):
                            try:
                                if os.path.exists(target_path):
                                    os.remove(target_path)
                                os.link(link_target, target_path)
                            except Exception:
                                # fallback: try extracting the file content
                                fileobj = tar.extractfile(member)
                                if fileobj is None:
                                    return False
                                with fileobj as src, open(target_path, "wb") as out_f:
                                    shutil.copyfileobj(src, out_f)
                        else:
                            # target not yet present; try extracting content directly
                            fileobj = tar.extractfile(member)
                            if fileobj is None:
                                return False
                            with fileobj as src, open(target_path, "wb") as out_f:
                                shutil.copyfileobj(src, out_f)
                        # Set permissions and mtime if available
                        try:
                            mode = member.mode & 0o777
                            os.chmod(target_path, mode)
                        except Exception:
                            pass
                        try:
                            if member.mtime is not None:
                                os.utime(target_path, (member.mtime, member.mtime))
                        except Exception:
                            pass
                    except Exception:
                        return False

                else:
                    # Unsupported/special file types (devices, FIFOs, etc.) -> treat as failure
                    return False

        return True

    except Exception:
        return False
