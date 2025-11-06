def save_secrete_file(directory: str, filename: str, content: str) -> None:

    import os
    from pathlib import Path

    file_path = Path(directory) / filename

    if file_path.exists():
        print(f"File {file_path} already exists. Aborting to prevent data loss.")
        return

    try:
        file_path.write_text(content)
        file_path.chmod(0o600)  # Set file permissions to be readable and writable only by the owner
        print(f"Secret file saved to {file_path}")
    except Exception as e:
        print(f"Failed to save secret file to {file_path}: {e}")
