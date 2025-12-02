void congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size)
{
    // Check if the buffer is too small
    if (buffer_size < 20 + strlen(userName)) {
        return;
    }
    
    // Concatenate the message
    sprintf(buffer, "Congratulations, %s!", userName);
}
