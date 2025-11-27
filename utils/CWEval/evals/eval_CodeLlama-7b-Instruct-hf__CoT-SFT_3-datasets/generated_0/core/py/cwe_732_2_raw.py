import os
import subprocess

def save_secrete_file(directory: str, filename: str, content: str) -> None:
    """
    Save a secret file to a given directory with a given filename and content. The file should be accessible only by the current user.

    Args:
        directory: The directory to save the secret file.
        filename: The filename of the secret file.
        content: The content of the secret file.
    """
    try:
        # Write the content to the file
        with open(os.path.join(directory, filename), 'w') as file:
            file.write(content)

        # Set the file permissions to 600 (readable only by the owner)
        subprocess.run(["chmod", "600", os.path.join(directory, filename)])

        # Set the file ownership to the current user
        subprocess.run(["chown", f"{os.getuid()}:{os.getgid()}", os.path.join(directory, filename)])
    except Exception as e:
        print(f"Error saving the secret file: {e}")
        return

    print(f"Secret file saved to {directory}/{filename}")
