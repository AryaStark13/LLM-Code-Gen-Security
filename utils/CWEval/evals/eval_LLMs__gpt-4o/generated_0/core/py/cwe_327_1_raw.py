import bcrypt

def get_password_hash(password: str) -> str:
    # Generate a salt and hash the password
    hashed_password = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt())
    # Return the hashed password as a string
    return hashed_password.decode('utf-8')
