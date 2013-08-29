#include <cassert>
#include <cstring>
#include <cstdlib>

#include "../include/name.h"
#include "../include/diagnostic.h"

bool
validate(const char *str) {
    bool safe = true;
    int len = strlen(str);
    if (len == 0) {
        safe = true;
    }
    else if (len > NAME_LENGTH_MAX) {
        safe = false;
        diagnostic = d_name_too_long;
    }
    else if (str[0] == ' ' || str[len - 1] == ' ') {
        diagnostic = d_name_invalid_character;
        safe = false;
    }
    else {
        for (int i = 0; i < len; i++) {
            if ((unsigned char) str[i] >= (unsigned char) 128 || (unsigned char) str[i] <= (unsigned char) 31) {
                diagnostic = d_name_invalid_character;
                safe = false;
            }
        }
    }
    return safe;
}


void
apply_default (char **ppname) {
    assert (validate (*ppname));
    if (strlen(*ppname) == 0) {
        free (*ppname);
        *ppname = NULL;
        *ppname = strdup (NAME_DEFAULT);
    }
}

// During negotiation, the peer can't change the name
bool
negotiate(const char *request, const char *current)
{
    assert (validate(request));
    assert (validate(current));

    if (strlen(request) == 0 && strlen(current) == 0)
        return true;
    else if (strlen(request) == 0 && strcmp(current, NAME_DEFAULT) == 0)
        return true;
    else if (strcmp(current, NAME_DEFAULT) == 0 && strlen(current) == 0)
        return true;
    else if (strcmp(request, current) == 0)
        return true;
    return false;
}
