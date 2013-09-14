#ifndef JOZA_WORKER_H
#define JOZA_WORKER_H
#include <czmq.h>
#include "lib.h"
#include "iodir.h"

#ifndef NAME_LEN
# define NAME_LEN 11U
#endif

typedef struct {
    bool valid;
    ukey_t key;
    const char *name;           /* not null-terminated */
    const void *zaddr;
    iodir_t io;
} worker_t;

typedef enum {READY, X_CALLER, Y_CALLEE} role_t;

void add_worker(zframe_t *A, const char *N, iodir_t I);
worker_t get_worker(ukey_t key);

#endif
