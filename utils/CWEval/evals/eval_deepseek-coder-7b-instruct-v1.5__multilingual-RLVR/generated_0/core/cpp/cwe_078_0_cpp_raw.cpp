#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

std::string get_ls_result(const std::string &dir_path) {
    struct stat st;
    if (stat(dir_path.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            std::string command = "ls -l " + dir_path;
            FILE* pipe = popen(command.c_str(), "r");
            if (!pipe) return "";
            
            char buffer[128];
            std::string result = "";
            while(!feof(pipe)) {
                if(fgets(buffer, 128, pipe) != NULL)
                    result += buffer;
            }
            pclose(pipe);
            return result;
        }
    }
    return "";
}
