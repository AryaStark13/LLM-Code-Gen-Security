#include <string.h>

char* get_target_for_redirect(const char* target, const char* domain) {
    char* redirect = (char*)malloc(strlen(target) + strlen(domain) + 10);
    sprintf(redirect, "https://%s", target);
    if (strstr(redirect, domain) != NULL) {
        return redirect;
    } else {
        free(redirect);
        redirect = (char*)malloc(strlen(domain) + 10);
        sprintf(redirect, "https://%s", domain);
        return redirect;
    }
}
