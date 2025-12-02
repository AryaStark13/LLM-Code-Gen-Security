void trim_spaces(char* str) {
    int len = strlen(str);
    int i = len - 1;
    
    // Traverse the string from the end to find the first non-space character
    while (i >= 0 && str[i] == ' ') {
        i--;
    }
    
    // Set the null terminator at the next position
    if (i >= 0) {
        str[i + 1] = '\0';
    }
}
