import os
import tarfile
import shutil
import posixpath
from typing import Optional, List, Tuple


def extract_tar_to_path(tar_path, dest_path) -> bool:
    """
    Extract the content of the tar file to the given destination path.
    If the extraction is successful, return True; otherwise, if any error occurs
    during the extraction process (e.g. file not found, permission error, cannot
    extract all content), just return False.

    Args:
        tar_path: the path to the tar file
        dest_path: the path to the destination folder

    Returns:
        True if the extraction is successful; otherwise, False
    """
    try:
        # Basic validations
        if not isinstance(tar_path, str) or not isinstance(dest_path, str):
            return False
        if not os.path.isfile(tar_path):
            return False
        if not tarfile.is_tarfile(tar_path):
            return False

        # Ensure destination directory exists
        try:
            os.makedirs(dest_path, exist_ok=True)
        except Exception:
            return False

        dest_abs = os.path.abspath(dest_path)

        # Helpers for path safety
        def _is_within_base(base: str, candidate_path: str) -> bool:
            """
            Verify candidate_path is within base using both abspath and realpath checks.
            """
            try:
                base_abs = os.path.abspath(base)
                cand_abs = os.path.abspath(candidate_path)
                # Quick abspath-based containment
                if os.path.commonpath([base_abs, cand_abs]) != base_abs:
                    return False
                # Realpath-based containment (resolving existing symlinks)
                cand_real = os.path.realpath(cand_abs)
                if os.path.commonpath([base_abs, cand_real]) != base_abs:
                    return False
                return True
            except Exception:
                return False

        def _target_path_for_member(name: str) -> str:
            """
            Compute the intended absolute filesystem path for a tar member name.
            """
            return os.path.abspath(os.path.join(dest_abs, name))

        def _symlink_target_abs_for_check(member_name: str, linkname: str) -> str:
            """
            Compute the absolute path (for safety checks) that a symlink would resolve to,
            with POSIX semantics of tar member paths, mapped into the destination root.
            """
            # Determine posix path of link target relative to member's directory
            try:
                if posixpath.isabs(linkname):
                    target_posix = posixpath.normpath(linkname)
                else:
                    # relative to the directory containing the symlink
                    parent_posix = posixpath.dirname(member_name)
                    target_posix = posixpath.normpath(posixpath.join(parent_posix, linkname))
                # Map this posix path into OS path space under dest_abs for checking
                # Note: if target_posix is absolute (starts with '/'), os.path.join will ignore base
                target_abs = os.path.abspath(os.path.join(dest_abs, target_posix))
                return target_abs
            except Exception:
                # On any error, return a path that will certainly fail containment
                return os.path.abspath(os.path.join(dest_abs, "..", "..", "..", "INVALID"))

        # Open the tar file
        try:
            tar = tarfile.open(tar_path, mode="r:*")
        except Exception:
            return False

        # Use context manager to ensure closure
        with tar:
            members = tar.getmembers()

            # We'll handle hardlinks after we have extracted other items
            deferred_hardlinks: List[Tuple[tarfile.TarInfo, str]] = []

            for member in members:
                try:
                    name = member.name
                    if not isinstance(name, str) or name == "":
                        return False

                    # Compute the final extraction path for this member
                    final_path = _target_path_for_member(name)

                    # Check that the target path is within destination
                    if not _is_within_base(dest_abs, final_path):
                        return False

                    # Handle directories
                    if member.isdir():
                        try:
                            os.makedirs(final_path, exist_ok=True)
                            # Optionally set directory permissions (ignore errors on unsupported OS)
                            try:
                                os.chmod(final_path, member.mode & 0o777)
                            except Exception:
                                pass
                            # Optionally set mtime for directory
                            try:
                                os.utime(final_path, (member.mtime, member.mtime))
                            except Exception:
                                pass
                        except Exception:
                            return False
                        continue

                    # Handle symlinks
                    if member.issym():
                        linkname = member.linkname or ""
                        # Ensure the symlink, when resolved in context, would still point within dest
                        link_target_abs = _symlink_target_abs_for_check(member.name, linkname)
                        if not _is_within_base(dest_abs, link_target_abs):
                            return False
                        # Ensure parent directory exists and is safe
                        parent_dir = os.path.dirname(final_path)
                        if not _is_within_base(dest_abs, parent_dir):
                            return False
                        try:
                            os.makedirs(parent_dir, exist_ok=True)
                        except Exception:
                            return False
                        # Create symlink
                        try:
                            # On some platforms, existing path must be removed before creating symlink
                            if os.path.lexists(final_path):
                                # Avoid removing if it's a directory; that would be dangerous
                                if os.path.islink(final_path) or os.path.isfile(final_path):
                                    try:
                                        os.remove(final_path)
                                    except Exception:
                                        return False
                                else:
                                    # Existing entity is a directory or special file; fail
                                    return False
                            os.symlink(member.linkname, final_path)
                            # Note: mtime for symlink is not reliably set in a cross-platform way
                        except Exception:
                            return False
                        continue

                    # Handle hardlinks (defer until after first pass)
                    if member.islnk():
                        # Validate the hardlink target path is within dest
                        target_abs = _target_path_for_member(member.linkname or "")
                        if not _is_within_base(dest_abs, target_abs):
                            return False
                        deferred_hardlinks.append((member, final_path))
                        continue

                    # Handle regular files
                    if member.isreg():
                        # Ensure parent directory exists and safe
                        parent_dir = os.path.dirname(final_path)
                        if not _is_within_base(dest_abs, parent_dir):
                            return False
                        try:
                            os.makedirs(parent_dir, exist_ok=True)
                        except Exception:
                            return False

                        # Additional safety: ensure that opening this path won't escape via symlink parents
                        # This is already covered by realpath check above.

                        # Extract file content
                        try:
                            source = tar.extractfile(member)
                            if source is None:
                                return False
                        except Exception:
                            return False

                        try:
                            # If existing path is a symlink or directory, fail
                            if os.path.islink(final_path):
                                return False
                            # Write file
                            with open(final_path, "wb") as out_f:
                                shutil.copyfileobj(source, out_f)
                        except Exception:
                            try:
                                # Attempt to remove partial file
                                if os.path.exists(final_path) and os.path.isfile(final_path):
                                    os.remove(final_path)
                            except Exception:
                                pass
                            return False
                        finally:
                            try:
                                if source:
                                    source.close()
                            except Exception:
                                pass

                        # Set permissions and modification time
                        try:
                            os.chmod(final_path, member.mode & 0o777)
                        except Exception:
                            pass
                        try:
                            os.utime(final_path, (member.mtime, member.mtime))
                        except Exception:
                            pass

                        continue

                    # Unsupported types (devices, fifos, etc.)
                    # For safety and portability, treat as failure
                    return False

                except Exception:
                    return False

            # Process deferred hardlinks now
            for hl_member, hl_path in deferred_hardlinks:
                try:
                    # Validate destination path still safe
                    if not _is_within_base(dest_abs, hl_path):
                        return False

                    # Compute the target of the hardlink and validate
                    target_abs = _target_path_for_member(hl_member.linkname or "")
                    if not _is_within_base(dest_abs, target_abs):
                        return False

                    # Target must exist and be a file
                    if not os.path.exists(target_abs) or not os.path.isfile(target_abs):
                        return False

                    # Ensure parent directory exists
                    parent_dir = os.path.dirname(hl_path)
                    if not _is_within_base(dest_abs, parent_dir):
                        return False
                    try:
                        os.makedirs(parent_dir, exist_ok=True)
                    except Exception:
                        return False

                    # If final path exists and is not a file, fail
                    if os.path.lexists(hl_path) and not os.path.isfile(hl_path):
                        return False
                    # Create hardlink if possible, otherwise fall back to copying contents
                    try:
                        # Remove existing file if present to create link cleanly
                        if os.path.isfile(hl_path):
                            try:
                                os.remove(hl_path)
                            except Exception:
                                return False
                        os.link(target_abs, hl_path)
                    except Exception:
                        # Fallback: copy file contents
                        try:
                            shutil.copy2(target_abs, hl_path)
                        except Exception:
                            return False

                    # Try to apply original metadata from tar info where possible
                    try:
                        os.chmod(hl_path, hl_member.mode & 0o777)
                    except Exception:
                        pass
                    try:
                        os.utime(hl_path, (hl_member.mtime, hl_member.mtime))
                    except Exception:
                        pass

                except Exception:
                    return False

        # If everything succeeded
        return True

    except Exception:
        # Any unexpected error should result in False
        return False
