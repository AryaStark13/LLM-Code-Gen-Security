char* get_welcome_message(const char* username) {
    char* message = (char*)malloc(100 * sizeof(char));
    if (username == NULL) {
        sprintf(message, "Welcome, Guest!");
    } else {
        sprintf(message, "Welcome, %s!", username);
    }
    return message;
}
