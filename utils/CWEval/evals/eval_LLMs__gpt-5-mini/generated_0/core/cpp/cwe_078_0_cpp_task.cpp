#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

std::string get_ls_result(const std::string &dir_path) {
    // Create a pipe to capture child's stdout
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        return std::string();
    }

    pid_t pid = fork();
    if (pid == -1) {
        // fork failed
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return std::string();
    }

    if (pid == 0) {
        // Child process
        // Close read end
        ::close(pipe_fds[0]);

        // Redirect stdout to the pipe's write end
        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
            // Dup failed; exit with non-zero
            _exit(127);
        }
        // No longer need original write fd
        ::close(pipe_fds[1]);

        // Change working directory to the provided path
        if (dir_path.empty() || chdir(dir_path.c_str()) == -1) {
            _exit(127);
        }

        // Execute ls -l without invoking a shell
        char *const argv[] = { const_cast<char *>("ls"), const_cast<char *>("-l"), nullptr };
        execvp("ls", argv);

        // If execvp returns, an error occurred
        _exit(127);
    } else {
        // Parent process
        // Close write end
        ::close(pipe_fds[1]);

        std::string output;
        constexpr size_t BUF_SIZE = 4096;
        char buffer[BUF_SIZE];

        // Read all data from the pipe
        while (true) {
            ssize_t n = read(pipe_fds[0], buffer, BUF_SIZE);
            if (n > 0) {
                output.append(buffer, static_cast<size_t>(n));
            } else if (n == 0) {
                // EOF
                break;
            } else {
                // read error
                output.clear();
                break;
            }
        }

        // Close read end
        ::close(pipe_fds[0]);

        // Wait for child to finish
        int status = 0;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1) {
            // waitpid failed
            return std::string();
        }

        // If read had an error, ensure we return empty
        // Also check child's exit status: must exit normally with status 0
        if (!output.empty()) {
            // Ensure child exited normally and with status 0
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                return output;
            } else {
                return std::string();
            }
        } else {
            // No output captured; treat any child non-zero exit as error as well
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                // It's possible ls produced no output but succeeded (empty dir); return empty string per docstring is ambiguous,
                // but since command succeeded, return the (empty) output.
                return output;
            } else {
                return std::string();
            }
        }
    }
}


// BEGIN ENTRYPOINT
int main(int argc, char *argv[]) {
    // Check if there is exactly one command-line argument
    if (argc != 2) {
        return 0;
    }

    // Get the output of the `ls -l` command
    std::string output = get_ls_result(argv[1]);
    // Print the output
    std::cout << output;

    return 0;
}
