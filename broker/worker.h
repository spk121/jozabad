#ifndef JOZA_WORKER_H
#define JOZA_WORKER_H

#include <czmq.h>
#include <assert.h>
#include <stdint.h>
#include "lib.h"
#include "iodir.h"
#include "ukey.h"
#include "../libjoza/joza_msg.h"

#ifdef COMPILE_WITHOUT_CZMQ
typedef void zframe_t;
#endif

#ifndef WORKER_COUNT
# define WORKER_COUNT 1024U
#endif
static_assert(WORKER_COUNT <= INT_MAX, "WORKER_COUNT too large");

#ifndef NAME_LEN
# define NAME_LEN 11U
#endif
static_assert(NAME_LEN <= INT8_MAX, "NAME_LEN too large");


typedef enum {READY, X_CALLER, Y_CALLEE} role_t;

extern uint32_t w_zhash[WORKER_COUNT];
extern char     w_name[WORKER_COUNT][NAME_LEN + 1];
extern zframe_t *w_zaddr[WORKER_COUNT];
extern iodir_t  w_iodir[WORKER_COUNT];
extern ukey_t   w_lcn[WORKER_COUNT];
extern role_t   w_role[WORKER_COUNT];

uint32_t addr2hash (const zframe_t *z);
uint32_t add_worker(const zframe_t *A, const char *N, iodir_t I);
bool_index_t get_worker(uint32_t key);
bool_t worker_dispatch_by_idx (joza_msg_t *M, size_t I);
void remove_worker_by_hash(uint32_t hash);
void remove_worker(uint32_t hash);
#endif
