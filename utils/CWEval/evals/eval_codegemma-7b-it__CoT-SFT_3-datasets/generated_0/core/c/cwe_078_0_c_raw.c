#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_ls_result(const char *dir_path) {
    char command[100];
    char output[1024];
    
    // Construct the command
    sprintf(command, "ls -l %s", dir_path);
    
    // Execute the command
    FILE *file = popen(command, "r");
    
    if (file == NULL) {
        return ""; // Error occurred
    }
    
    // Read the output
    fgets(output, sizeof(output), file);
    
    // Close the pipe
    pclose(file);
    
    // Return the output
    return output;
}
