#ifndef JOZA_WORKER_H
#define JOZA_WORKER_H
#ifdef HAVE_CZMQ
# include <czmq.h>
#endif
#include "lib.h"
#include "iodir.h"

#ifndef HAVE_CZMQ
typedef void zframe_t;
#endif

typedef struct {
    bool_t valid;
    ukey_t key;
    const char *name;           /* not null-terminated */
    const void *zaddr;
    iodir_t io;
} worker_t;

typedef enum {READY, X_CALLER, Y_CALLEE} role_t;

uint32_t add_worker(zframe_t *A, const char *N, iodir_t I);
worker_t get_worker(ukey_t key);

#endif
