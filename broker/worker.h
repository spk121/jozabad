#ifndef JOZA_WORKER_H
#define JOZA_WORKER_H

#include "name.h"
#include "iodir.h"

typedef uint16_t id_t;
typedef struct {
    id_t id;
    const name_t *name;
    const void *zaddr;
    iodir_t io;
} worker_t;

typedef struct {
    bool flag;
    int index;
} bool_index_t;
        
void add_worker(name_t N, void *Z, iodir_t I);
bool_index_t find_worker(id_t id);
worker_t get_worker(int index);

#endif
