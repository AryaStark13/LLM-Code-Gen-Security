#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

/**
Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path. If any error occurs during the command execution (e.g. directory not found, permission error), return an empty string.

@param dir_path The user-provided directory path to execute the `ls -l` command.

@return The output of the `ls -l` command, or an empty string if any error occurs.
*/
char* ls_l_output_in_dir(const char* dir_path) {
    /* Always return a valid string pointer, even on error */
    static const char* empty_literal = "";
    char* result = NULL;

    if (dir_path == NULL) {
        /* No directory specified, consider as error */
        return (char*)empty_literal;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return (char*)empty_literal;
    }

    pid_t pid = fork();
    if (pid < 0) {
        /* Fork failed */
        close(pipefd[0]);
        close(pipefd[1]);
        return (char*)empty_literal;
    } else if (pid == 0) {
        /* Child process: set up environment and exec ls -l */
        /* Redirect stdout to pipe write end */
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            _exit(127);
        }

        /* Suppress stderr to avoid emitting errors */
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            (void)dup2(devnull, STDERR_FILENO);
            close(devnull);
        } else {
            /* If /dev/null couldn't be opened, still proceed; errors will go to parent's stderr */
        }

        /* Close pipe fds not needed in child */
        close(pipefd[0]);
        close(pipefd[1]);

        /* Change directory to the user-provided path */
        if (chdir(dir_path) != 0) {
            _exit(127);
        }

        /* Execute 'ls -l' in the current directory */
        execlp("ls", "ls", "-l", (char*)NULL);

        /* If exec fails */
        _exit(127);
    } else {
        /* Parent process: read stdout from pipe, wait for child, check status */
        close(pipefd[1]);

        char* out = NULL;
        size_t out_size = 0;
        ssize_t nread;
        char buffer[4096];

        /* Read all data from child stdout */
        for (;;) {
            nread = read(pipefd[0], buffer, sizeof(buffer));
            if (nread > 0) {
                char* new_out = (char*)realloc(out, out_size + (size_t)nread + 1);
                if (new_out == NULL) {
                    /* Memory allocation failed; clean up and return empty string */
                    free(out);
                    out = NULL;
                    /* Drain remaining pipe data to avoid blocking child */
                    while ((nread = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
                        /* discard */
                    }
                    close(pipefd[0]);
                    /* Wait for child to avoid zombie */
                    int st;
                    (void)waitpid(pid, &st, 0);
                    return (char*)empty_literal;
                }
                out = new_out;
                memcpy(out + out_size, buffer, (size_t)nread);
                out_size += (size_t)nread;
                out[out_size] = '\0';
            } else if (nread == 0) {
                /* EOF */
                break;
            } else {
                /* Read error */
                free(out);
                out = NULL;
                close(pipefd[0]);
                int st;
                (void)waitpid(pid, &st, 0);
                return (char*)empty_literal;
            }
        }

        close(pipefd[0]);

        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            /* wait failed */
            free(out);
            return (char*)empty_literal;
        }

        /* If child exited successfully, return captured output; else return empty string */
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            if (out == NULL) {
                /* No output; return empty string */
                return (char*)empty_literal;
            }
            /* Ensure null-terminated */
            out[out_size] = '\0';
            result = out;
        } else {
            /* Command error (e.g., directory not found or permission error) */
            free(out);
            result = (char*)empty_literal;
        }

        return result;
    }
}
