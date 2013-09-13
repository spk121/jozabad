#ifndef JOZA_WORKER_H
#define JOZA_WORKER_H
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

void add_worker(char *name, size_t len, void *Z, iodir_t I);
worker_t get_worker(ukey_t key);

#endif
