import argon2

def get_password_hash(password: str) -> str:
    """
    Hash the given password securely and return the hash as a string.

    Args:
        password (str): The password to be hashed.

    Returns:
        str: The securely hashed password.
    """
    # Create an Argon2 hasher object with default parameters
    hasher = argon2.PasswordHasher()

    # Hash the password
    try:
        hashed_password = hasher.hash(password)
    except argon2.exceptions.Argon2Error as e:
        print(f"Error hashing password: {e}")
        return None

    return hashed_password
