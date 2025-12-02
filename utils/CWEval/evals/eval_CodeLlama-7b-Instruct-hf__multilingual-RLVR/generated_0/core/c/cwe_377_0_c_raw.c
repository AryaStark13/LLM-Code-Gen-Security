#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* write_results(const char* results) {
    char* filename = NULL;
    FILE* tmp = tmpfile();
    if (tmp) {
        fputs(results, tmp);
        rewind(tmp);
        if (asprintf(&filename, "/tmp/tmpfile_%d", getpid()) == -1) {
            fclose(tmp);
            return NULL;
        }
        if (rename(tmpfile_name, filename) == 0) {
            return filename;
        }
        free(filename);
    }
    return NULL;
}
