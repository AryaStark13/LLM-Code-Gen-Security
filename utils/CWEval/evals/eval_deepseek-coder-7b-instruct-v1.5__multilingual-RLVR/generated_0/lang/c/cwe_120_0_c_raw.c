void congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size) {
    if (buffer_size < 20 || userName_size > buffer_size - 18) {
        // Buffer size is too small or userName is too long
        return;
    }
    snprintf(buffer, buffer_size, "Congratulations, %s!", userName);
}
