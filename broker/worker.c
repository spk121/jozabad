#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"
#include "worker.h"
#include "iodir.h"

#ifndef WORKER_COUNT
# define WORKER_COUNT 1024U
#endif
static_assert(WORKER_COUNT <= INT_MAX, "WORKER_COUNT too large");

#ifndef NAME_LEN
# define NAME_LEN 11U
#endif
static_assert(NAME_LEN <= INT8_MAX, "NAME_LEN too large");

/*
  These arrays form a hash table whose data is keyed either by
  a string hash of the ZMQ address or by name.

  zhash uint32_t   [primary key]    a hash created from the ZMQ Router identity
  name  char[17]   [secondary key]  a string name for the connection
  zaddr zframe_t *                  a ZMQ frame containing a ZMQ Router identity
  iodir iodir_t                     whether this worker makes or receives calls
  lcn   lcn_t                       Logical Channel Number
  role  role_t                      X (caller), Y (callee), or READY
  ctime uint64_t                    time this worker was created
  atime uint64_t                    time of last message from this worker
  mtime uint64_t                    time of last modification to
                                       hash/name/addr/iodir/lcn/role

  nidx  size_t     [secondary key]  an array that keeps the sort indices
                                    for the secondary key
*/

// Number of workers
static size_t   _count = 0;

// Data for the workers
static uint32_t w_zhash[WORKER_COUNT];
static char     w_name[WORKER_COUNT][NAME_LEN + 1];
static void     *w_zaddr[WORKER_COUNT];
static iodir_t  w_iodir[WORKER_COUNT];
static ukey_t   w_lcn[WORKER_COUNT];
static role_t   w_role[WORKER_COUNT];
static double   w_ctime[WORKER_COUNT];
static double   w_atime[WORKER_COUNT];
static double   w_mtime[WORKER_COUNT];

// List of the sort order of the strings in w_name
static size_t   w_nidx[WORKER_COUNT];
// FIXME: PEDANTIC - pointers to the names in w_name;
static char     *w_pname[WORKER_COUNT];

void init_pname(void)
{
    for (size_t i = 0; i < WORKER_COUNT; i ++)
        w_pname[i] = &(w_name[i][0]);
}

// Deep diving into the ZeroMQ source says that for ZeroMQ 3.x,
// bytes 1 to 5 of the address would work as a unique ID for a
// given connection.
static uint32_t addr2hash (zframe_t *z)
{
    uint32_t x[1];
    memcpy(x, (char *) zframe_data(z) + 1, sizeof(uint32_t));
    return x[0];
}

static void push
#define PUSH(arr,idx,count)  _Generic((a                                 \
    memmove(arr + idx + 1, arr + idx, sizeof(arr[0]) * (count - idx))

static void pushd(double arr[], size_t idx, size_t count)
{
    if (PARANOIA)
    {
        for (int i = _count; i >= idx + 1; i --)
        {
            arr[i] = arr[i-1];
        }
        arr[idx] = 0.0;
    }
    else
        memmove(&arr[idx+1], &arr[idx], sizeof(double) * (count - idx));
}

// Add a new worker to the store.  Returns true on success or false
// on failure.  Everything is supposed to be pre-validated, so 
// invalid means that this message is ignored.
uint32_t add_worker(zframe_t *A, const char *N, iodir_t I)
{
    uint32_t hash;
    size_t i;
    double elapsed_time = now();

    if (_count >= WORKER_COUNT)
        return;

    hash = addr2hash(A);
    if (hash == 0)
        return;

    i = ifind(w_zhash, _count, hash);
    if (w_zhash[i] == hash)
        return;

    if (strnlen_s(N, NAME_LEN + 1) > NAME_LEN
        || !safeascii(N, NAME_LEN))
        return;
    if (!VAL_IODIR(I))
        return;
    
    if (i < _count) {
        PUSH(w_zhash, i, _count);
        PUSH(w_zaddr, i, _count);
        PUSH(w_name, i, _count);
        PUSH(w_iodir, i, _count);
        PUSH(w_lcn, i, _count);
        PUSH(w_role, i, _count);
        pushd(w_ctime, i, _count);
        pushd(w_atime, i, _count);
        pushd(w_mtime, i, _count);
    }

    w_zhash[i] = hash;
    memset(w_name[i], 0, NAME_LEN + 1);
    strncpy(w_name[i], N, NAME_LEN);
    w_zaddr[i] = A;
    w_iodir[i] = I;
    w_lcn[i] = UKEY_C(0);
    w_role[i] = READY;
    
    w_ctime[i] = elapsed_time;
    w_mtime[i] = elapsed_time;
    w_atime[i] = elapsed_time;

    // Update the index table that alphabetizes the names.
    qisort(w_pname, _count, w_nidx);
    return hash;
}

void remove_worker(uint32_t hash)
{
    if (_count == 0)
        return;
    i = ifind(w_zhash, _count, hash);
    if (w_zhash[i] != hash)
        return;
    if (i < _count - 1)
    {
        REMOVE(w_zhash, i, _count);
        REMOVE(w_zaddr, i, _count);
        REMOVE(w_name, i, _count);
        REMOVE(w_iodir, i, _count);
        REMOVE(w_lcn, i, _count);
        REMOVE(w_role, i, _count);
        removed(w_ctime, i, _count);
        removed(w_atime, i, _count);
        removed(w_mtime, i, _count);
    }
    _count --;
    w_zhash[_count] = 0;
    memset(w_name[_count], 0, NAME_LEN + 1);
    w_zaddr[_count] = NULL;
    w_iodir[i] = BIDIRECTIONAL;
    w_lcn[i] = UKEY_C(0);
    w_role[i] = READY;
    w_ctime[i] = -1.0;
    w_mtime[i] = -1.0;
    w_atime[i] = -1.0;
}

#undef REMOVE
