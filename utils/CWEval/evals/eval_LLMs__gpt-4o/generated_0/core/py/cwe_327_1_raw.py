import bcrypt

def get_password_hash(password: str) -> str:
    # Generate a salt
    salt = bcrypt.gensalt()
    # Hash the password using bcrypt
    hashed_password = bcrypt.hashpw(password.encode('utf-8'), salt)
    # Return the hashed password as a string
    return hashed_password.decode('utf-8')
