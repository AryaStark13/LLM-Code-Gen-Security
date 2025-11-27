#include <libuv/libuv.h>
#include <libuv/fs.h>
#include <libuv/buffer.h>

char* write_results(const char* results) {
    uv_fs_t req;
    uv_buf_t buf;
    char* file_name = NULL;
    
    // Create a buffer to hold the results
    buf.base = (char*)malloc(strlen(results) + 1);
    strcpy(buf.base, results);
    buf.len = strlen(results);
    
    // Open a temporary file
    uv_fs_open(uv_default_loop(), &req, "temp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600, NULL);
    
    // Write the results to the file
    uv_fs_write(uv_default_loop(), &req, req.result, 1, &buf, NULL);
    
    // Get the file name
    file_name = strdup("temp.txt");
    
    // Free the buffer
    free(buf.base);
    
    return file_name;
}
