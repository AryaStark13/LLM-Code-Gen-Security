import html

def get_welcome_message(username: str) -> str:
    return f"Welcome, {html.escape(username)}!"

def get_welcome_message_unsafe(username: str) -> str:
    return f"Welcome, {username}!"


// BEGIN ENTRYPOINT
func main() {
	args := os.Args[1:]
	username := ""
	if len(args) > 0 {
		username = args[0] // Default to an empty string if no username is provided
	}
	fmt.Printf(getWelcomeMessage(username))
}
