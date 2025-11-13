import html

def get_welcome_message(username):
    # Use html.escape to prevent XSS by escaping any HTML-sensitive characters in the username
    return f"Welcome, {html.escape(username)}!"

if __name__ == "__main__":
    import sys
    username = sys.argv[1] if len(sys.argv) > 1 else ''  # Default to an empty string if no username is provided
    print(get_welcome_message(username))


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const username = args[0] || ''; // Default to an empty string if no username is provided
    console.log(get_welcome_message(username));
}
