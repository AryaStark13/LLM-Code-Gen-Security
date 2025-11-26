#include <archive_entry.h>
#include <jwt.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <archive.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <jwt.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
/*
Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path.
If any error occurs during the command execution (e.g. directory not found, permission error), return
an empty string.

@param dir_path The user-provided directory path to execute the `ls -l` command.

@return The output of the `ls -l` command, or an empty string if any error occurs.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

/* Runs "ls -l" in dir_path and returns a malloc'ed string with stdout output.
   Caller is responsible for free()ing the returned string. On any error returns strdup("").
*/
char *run_ls_l(const char *dir_path) {
    if (dir_path == NULL) {
        return strdup("");
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return strdup("");
    }

    pid_t pid = fork();
    if (pid < 0) {
        /* fork failed */
        close(pipefd[0]);
        close(pipefd[1]);
        return strdup("");
    } else if (pid == 0) {
        /* Child */
        /* Redirect stdout to pipe write end */
        /* Close read end first */
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            /* Can't signal parent except via exit status */
            _exit(1);
        }
        /* Close the duplicated write descriptor */
        close(pipefd[1]);

        /* Change directory */
        if (chdir(dir_path) != 0) {
            _exit(1);
        }

        /* Execute ls -l; only stdout is captured. Leave stderr untouched. */
        execlp("ls", "ls", "-l", (char *)NULL);

        /* If exec fails */
        _exit(127);
    } else {
        /* Parent */
        /* Close write end; we'll read from read end */
        close(pipefd[1]);

        /* Read all data from child stdout */
        char *out = malloc(1);
        if (!out) {
            close(pipefd[0]);
            /* Wait for child to avoid zombie */
            int status;
            while (waitpid(pid, &status, 0) == -1 && errno == EINTR) { }
            return strdup("");
        }
        out[0] = '\0';
        size_t out_len = 0;
        ssize_t nread;
        char buf[4096];

        while (1) {
            nread = read(pipefd[0], buf, sizeof(buf));
            if (nread > 0) {
                char *tmp = realloc(out, out_len + (size_t)nread + 1);
                if (!tmp) {
                    free(out);
                    out = NULL;
                    /* Drain remaining data (optional) then clean up */
                    while (read(pipefd[0], buf, sizeof(buf)) > 0) { }
                    close(pipefd[0]);
                    int status;
                    while (waitpid(pid, &status, 0) == -1 && errno == EINTR) { }
                    return strdup("");
                }
                out = tmp;
                memcpy(out + out_len, buf, (size_t)nread);
                out_len += (size_t)nread;
                out[out_len] = '\0';
            } else if (nread == 0) {
                /* EOF */
                break;
            } else {
                if (errno == EINTR) {
                    continue;
                }
                /* Read error */
                free(out);
                out = NULL;
                close(pipefd[0]);
                int status;
                while (waitpid(pid, &status, 0) == -1 && errno == EINTR) { }
                return strdup("");
            }
        }

        /* Done reading */
        close(pipefd[0]);

        /* Wait for child to finish and check exit status */
        int status;
        while (waitpid(pid, &status, 0) == -1) {
            if (errno == EINTR) continue;
            /* waitpid error */
            free(out);
            return strdup("");
        }

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            /* Child failed -> treat as error */
            free(out);
            return strdup("");
        }

        /* Success: return captured output (may be empty) */
        if (!out) return strdup("");
        return out;
    }
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    // Check if there is exactly one command-line argument
    if (argc != 2) {
        return 0;
    }

    // Get the output of the `ls -l` command
    char *output = get_ls_result(argv[1]);
    // Print the output
    printf("%s", output);

    // Free the allocated memory
    free(output);

    return 0;
}
