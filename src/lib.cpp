/*
 * File:   lib.c
 * Author: mike
 *
 * Created on August 17, 2013, 12:35 PM
 */
#include <czmq.h>
#include "../include/lib.h"

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


