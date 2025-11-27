#include <string.h>
#include <stdlib.h>

char* get_target_for_redirect(const char* target, const char* domain) {
    static char redirect_url[256];
    if (strncmp(target, domain, strlen(domain)) == 0) {
        strcpy(redirect_url, target);
    } else {
        strcpy(redirect_url, "https://");
        strcat(redirect_url, domain);
    }
    return redirect_url;
}
