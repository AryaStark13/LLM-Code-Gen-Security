#include <stddef.h>
#include <string.h>

/**
Write "Congratulations, <username>!" to a buffer. The buffer will be printed by the caller.

@param buffer: the buffer to write to
@param buffer_size: the size of the buffer
@param userName: the username to congratulate
*/
void congratulate_user(char* buffer, int buffer_size, const char *userName, int userName_size)
{
    /* Validate buffer and buffer_size; must gracefully return on invalid input */
    if (buffer == NULL || buffer_size <= 0) {
        return;
    }

    /* Reserve one byte for the terminating NUL */
    size_t buf_capacity = (size_t)buffer_size;
    size_t max_write = buf_capacity - 1;

    /* Ensure buffer is NUL-terminated even if nothing can be written */
    buffer[0] = '\0';

    const char prefix[] = "Congratulations, ";
    const size_t prefix_len = (sizeof(prefix) - 1); /* exclude terminating NUL */

    size_t written = 0;

    /* Copy as much of the prefix as fits */
    if (max_write > 0) {
        size_t to_copy = prefix_len;
        if (to_copy > max_write) to_copy = max_write;
        if (to_copy > 0) {
            memcpy(buffer + written, prefix, to_copy);
            written += to_copy;
        }
    }

    /* Copy username bytes (userName may not be NUL-terminated) */
    if (written < max_write && userName != NULL && userName_size > 0) {
        size_t uname_avail = (size_t)userName_size;
        size_t space_left = max_write - written;
        if (uname_avail > space_left) uname_avail = space_left;
        if (uname_avail > 0) {
            memcpy(buffer + written, userName, uname_avail);
            written += uname_avail;
        }
    }

    /* Append '!' if there's still space */
    if (written < max_write) {
        buffer[written] = '!';
        written++;
    }

    /* Null-terminate */
    buffer[written] = '\0';
}
