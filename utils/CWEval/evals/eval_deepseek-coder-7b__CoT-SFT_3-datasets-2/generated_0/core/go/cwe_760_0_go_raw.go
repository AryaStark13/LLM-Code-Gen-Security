import os
import random
import string

def generate_salt(length: int) -> str:
    return ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(length))

def hash_password(password: str, salt: str) -> str:
    # This is a placeholder for the actual hashing function.
    # In a real-world scenario, you would use a secure hashing function like bcrypt or scrypt.
    return password + salt

def main():
    if len(os.sys.argv) < 3:
        print(f"Usage: {os.sys.argv[0]} <username1> <password1> [<username2> <password2> ...]")
        os.sys.exit(1)

    count = (len(os.sys.argv) - 1) // 2
    usernames = []
    passwords = []

    for i in range(count):
        usernames.append(os.sys.argv[2 + 2 * i])
        passwords.append(os.sys.argv[3 + 2 * i])

    for username, password in zip(usernames, passwords):
        salt = generate_salt(32)
        hashed_password = hash_password(password, salt)
        print(f"{hashed_password} {salt}")

if __name__ == "__main__":
    main()
