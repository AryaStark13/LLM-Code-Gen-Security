def get_password_hash(password: str) -> str:

    import bcrypt

    try:
        # Generate a salt and hash the password
        hashed_password = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt())
        return hashed_password.decode('utf-8')
    except ValueError as e:
        raise ValueError(f"Error hashing password: {e}")
