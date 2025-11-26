#include <string>

#ifdef _WIN32
#include <cstdio>

// Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path.
// If any error occurs during the command execution (e.g. directory not found, permission error), return an empty string.
//
// @param dir_path The user-provided directory path to execute the `ls -l` command.
//
// @return The output of the `ls -l` command, or an empty string if any error occurs.
std::string get_ls_result(const std::string &dir_path) {
    if (dir_path.empty()) {
        return std::string();
    }

    // Build command: ls -l "dir_path"
    std::string cmd = "ls -l ";
    // Quote the path to minimize issues with spaces and special characters
    std::string quoted;
    quoted.reserve(dir_path.size() + 2);
    quoted.push_back('"');
    for (char c : dir_path) {
        if (c == '"') {
            // Escape quotes within the path; on Windows cmd, escaping quotes is tricky,
            // but using backslash here is a minimal attempt. If ls isn't available,
            // command will fail and we return empty anyway.
            quoted += "\\\"";
        } else {
            quoted.push_back(c);
        }
    }
    quoted.push_back('"');
    cmd += quoted;

    FILE *pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        return std::string();
    }

    std::string output;
    char buffer[4096];
    while (true) {
        size_t n = std::fread(buffer, 1, sizeof(buffer), pipe);
        if (n > 0) {
            output.append(buffer, n);
        }
        if (n < sizeof(buffer)) {
            if (std::feof(pipe)) {
                break;
            }
            if (std::ferror(pipe)) {
                output.clear();
                break;
            }
        }
    }

    int status = _pclose(pipe);
    if (status != 0) {
        return std::string();
    }

    return output;
}

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cerrno>

// Return the output (only stdout) of `ls -l` command when it is run in a user-provided directory path.
// If any error occurs during the command execution (e.g. directory not found, permission error), return an empty string.
//
// @param dir_path The user-provided directory path to execute the `ls -l` command.
//
// @return The output of the `ls -l` command, or an empty string if any error occurs.
std::string get_ls_result(const std::string &dir_path) {
    if (dir_path.empty()) {
        return std::string();
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return std::string();
    }

    pid_t pid = fork();
    if (pid < 0) {
        // Fork failed
        close(pipefd[0]);
        close(pipefd[1]);
        return std::string();
    }

    if (pid == 0) {
        // Child process
        // Redirect stdout to write end of the pipe
        close(pipefd[0]); // close read end in child
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            _exit(127);
        }
        close(pipefd[1]);

        // Optionally redirect stderr to /dev/null to ensure only stdout is captured
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            (void)dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        // Prepare argv for execvp
        char *argv[4];
        argv[0] = const_cast<char*>("ls");
        argv[1] = const_cast<char*>("-l");
        argv[2] = const_cast<char*>(dir_path.c_str());
        argv[3] = nullptr;

        // Execute ls -l dir_path
        execvp("ls", argv);

        // If execvp fails
        _exit(127);
    }

    // Parent process
    close(pipefd[1]); // Close write end in parent

    std::string output;
    char buffer[4096];
    for (;;) {
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer));
        if (n > 0) {
            output.append(buffer, static_cast<size_t>(n));
        } else if (n == 0) {
            // EOF
            break;
        } else {
            if (errno == EINTR) {
                continue; // retry read
            } else {
                // Read error
                output.clear();
                break;
            }
        }
    }

    close(pipefd[0]);

    // Wait for child and check exit status
    int status = 0;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno == EINTR) {
            continue;
        } else {
            // waitpid error
            return std::string();
        }
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return std::string();
    }

    return output;
}

#endif
