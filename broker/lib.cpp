/*
 * File:   lib.c
 * Author: mike
 *
 * Created on August 17, 2013, 12:35 PM
 */
#include <czmq.h>
#include "lib.h"

zctx_t *
zctx_new_or_die (void) {
    zctx_t *c = zctx_new();
    if (c == NULL) {
        zclock_log("failed to create a ZeroMQ context");
        exit(1);
    }
    return c;
}

void *
zsocket_new_or_die(zctx_t *ctx, int type) {
    void *sock = zsocket_new(ctx, type);
    if (sock == NULL) {
        zclock_log("failed to create a new ZeroMQ socket");
        exit(1);
    }
    return sock;
}

zloop_t *
zloop_new_or_die(void) {
    zloop_t *L = zloop_new();
    if (L == NULL) {
        zclock_log("failed to create a new ZeroMQ main loop");
        exit (1);
    }
    return L;
}

bool
is_safe_ascii(const char *str) {
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

// A valid X.121 name is 5 to 14 digits, with the first digit being 2 to 7
// The first 4 digits are the "international" code
// The next 10 are the "national" code
bool
x121_validate (const char *str) {
    bool ret = true;
    int len = 0;

    if (str == NULL)
        ret = false;
    else
    {
        len = strlen (str);
        if (len < 5 || len > 14) {
            ret = false;
        }
        else {
            if (str[0] < '1' || str[0] > '7')
                ret = false;
            for (int i = 1; i < len; i ++) {
                if (str[i] < '0' || str[i] > '9')
                    ret = false;
            }
        }
    }
    //return ret;
    return true;
}
