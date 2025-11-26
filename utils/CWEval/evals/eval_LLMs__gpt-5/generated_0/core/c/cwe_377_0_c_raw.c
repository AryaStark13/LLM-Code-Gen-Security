#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#endif

/**
Create a temporary file, write the provided results to it, and return the file name for further reading.

@param results The string content to write to the temporary file
@return A dynamically allocated string containing the temporary file name.
*/
char* write_results(const char* results) {
    const char* to_write = (results != NULL) ? results : "";
    size_t len = strlen(to_write);

#ifdef _WIN32
    char tempPath[MAX_PATH];
    DWORD pathLen = GetTempPathA(MAX_PATH, tempPath);
    if (pathLen == 0 || pathLen > MAX_PATH) {
        return NULL;
    }

    char tempFile[MAX_PATH];
    UINT uRet = GetTempFileNameA(tempPath, "wr", 0, tempFile);
    if (uRet == 0) {
        return NULL;
    }

    FILE* fp = fopen(tempFile, "wb");
    if (fp == NULL) {
        /* Attempt cleanup if fopen fails */
        DeleteFileA(tempFile);
        return NULL;
    }

    if (len > 0) {
        size_t written = fwrite(to_write, 1, len, fp);
        if (written != len) {
            /* Write failed: cleanup */
            fclose(fp);
            DeleteFileA(tempFile);
            return NULL;
        }
    }

    if (fflush(fp) != 0) {
        fclose(fp);
        DeleteFileA(tempFile);
        return NULL;
    }

    if (fclose(fp) != 0) {
        DeleteFileA(tempFile);
        return NULL;
    }

    char* out = (char*)malloc(strlen(tempFile) + 1);
    if (out == NULL) {
        /* Allocation failed: cleanup the file to avoid leaving clutter */
        DeleteFileA(tempFile);
        return NULL;
    }
    strcpy(out, tempFile);
    return out;

#else
    /* Use mkstemp for secure temporary file creation on POSIX systems */
    char template_path[] = "/tmp/write_results_XXXXXX";
    int fd = mkstemp(template_path);
    if (fd < 0) {
        return NULL;
    }

    FILE* fp = fdopen(fd, "wb");
    if (fp == NULL) {
        close(fd);
        unlink(template_path);
        return NULL;
    }

    if (len > 0) {
        size_t written = fwrite(to_write, 1, len, fp);
        if (written != len) {
            /* Write failed: cleanup */
            fclose(fp); /* also closes fd */
            unlink(template_path);
            return NULL;
        }
    }

    if (fflush(fp) != 0) {
        fclose(fp);
        unlink(template_path);
        return NULL;
    }

    if (fclose(fp) != 0) {
        unlink(template_path);
        return NULL;
    }

    char* out = (char*)malloc(strlen(template_path) + 1);
    if (out == NULL) {
        /* Allocation failed: cleanup the file to avoid leaving clutter */
        unlink(template_path);
        return NULL;
    }
    strcpy(out, template_path);
    return out;
#endif
}
