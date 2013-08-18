/*
 * File:   lib.c
 * Author: mike
 *
 * Created on August 17, 2013, 12:35 PM
 */

#include <string.h>
#include <stdbool.h>

bool
is_safe_ascii(const char * const str) {
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


