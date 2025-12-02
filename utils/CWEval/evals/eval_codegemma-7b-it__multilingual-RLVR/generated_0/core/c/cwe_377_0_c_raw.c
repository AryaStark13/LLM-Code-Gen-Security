#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

char* write_results(const char* results) {
    // Create a temporary file
    char* temp_file_name = malloc(256 * sizeof(char));
    if (temp_file_name == NULL) {
        return NULL;
    }
    
    // Write the results to the temporary file
    int file_descriptor = open(temp_file_name, O_WRONLY | O_CREAT, 0600);
    if (file_descriptor < 0) {
        free(temp_file_name);
        return NULL;
    }
    
    if (write(file_descriptor, results, strlen(results)) < 0) {
        close(file_descriptor);
        free(temp_file_name);
        return NULL;
    }
    
    close(file_descriptor);
    
    return temp_file_name;
}
