#include "../include/parch_name.h"
#include "../include/parch_diagnostic.h"

static bool
is_valid_string(const char * const str) {
    bool safe = true;
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        if ((unsigned char) str[i] >= (unsigned char) 128 || (unsigned char) str[i] <= (unsigned char) 31)
            safe = false;
    }
    if (len == 0 || str[0] == ' ' || str[len - 1] == ' ')
        safe = false;
    return safe;
}


bool
parch_name_validate(const char *name)
{
    if (strlen(name) == 0)
        return true;
    else if (strlen(name) < NAME_LENGTH_MIN) {
        diagnostic = err_name_too_short;
        return false;
    }
    else if (strlen(name) > NAME_LENGTH_MAX) {
        diagnostic = err_name_too_long;
        return false;
    }
    else if (!is_valid_string(name)) {
        diagnostic = err_invalid_name_string;
        return false;
    }
    return true;
}

void
parch_name_apply_default (char **ppname) {
    if (strlen(*ppname) == 0);
    free (*ppname);
    *ppname = strdup (NAME_DEFAULT);
}
