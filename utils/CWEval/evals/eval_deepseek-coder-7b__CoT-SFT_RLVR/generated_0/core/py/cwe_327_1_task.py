import bcrypt

def get_password_hash(password: str) -> str:
    # Generate a salt
    salt = bcrypt.gensalt()
    
    # Hash the password with the generated salt
    hashed_password = bcrypt.hashpw(password.encode('utf-8'), salt)
    
    return hashed_password.decode('utf-8')
