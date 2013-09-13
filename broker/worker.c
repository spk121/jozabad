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
# define WORKER_COUNT UKEY_C(1024)
#endif
static_assert(WORKER_COUNT < UKEY_MAX, "WORKER_COUNT too large for ukey_t");

/*
  These arrays form a hash table whose data is keyed either by
  a string hash of the ZMQ address or by name.
*/

/*
zhash uint32_t   [primary key]    a hash created from the ZMQ Router identity
name  char[16]   [secondary key]  a string name for the connection
zaddr zframe_t *                  a ZMQ frame containing a ZMQ Router identity                     
iodir iodir_t                     whether this worker makes or receives calls
lcn   lcn_t                       Logical Channel Number
role  role_t                      X (caller), Y (callee), or READY (not in a call)
ctime uint64_t                   time this worker was created
atime uint64_t                   time of last message from this worker
mtime uint64_t                    time of last modification to hash/name/addr/iodir/lcn/role

nidx  size_t     [secondary key]  an array that keeps the sort indices for the secondary key
*/                                                                

                                                                

static size_t _count = 0;
static ukey_t _key = 0;
static ukey_t w_key[WORKER_COUNT];
static char w_name[WORKER_COUNT][NAME_LEN +1];
static uint8_t w_zaddr[WORKER_COUNT][ZADDR_LEN];
static iodir_t w_iodir[WORKER_COUNT];


#define PUSH(arr,idx,count)                                             \
    memmove(arr + idx + 1, arr + idx, sizeof(arr[0]) * (count - idx))

/* Given a name N, and Zmq address Z, and a I/O direction I,
   - finds a free ID number to identify this new worker
   - adds this new worker into the list of workers
   - keeps the sorting of the list in order of ID */

void add_worker(char *str, size_t len, void *Z, iodir_t I)
{
    assert(_count < WORKER_COUNT);
    assert(VAL_IODIR(I));
    assert(str != NULL);
    assert(len <= NAME_LEN);

    index_ukey_t kid = keynext(w_key, _count, _key);
    size_t idx = kid.index;
    ukey_t key = key;

    if (idx < _count)
    {
        PUSH(w_key, idx, _count);
        PUSH(w_zaddr, idx, _count);
        PUSH(w_iodir, idx, _count);
        PUSH(w_name, idx, _count);
    }
    w_key[idx] = key;
    memset(&w_name[idx][0], 0, NAME_LEN +1);
    memcpy(w_name[idx], str, len);
    w_zaddr[idx] = Z;
    w_iodir[idx] = I;

    _count ++;
    _key = key;
}
#undef PUSH

// Returns information about the worker with the given KEY.  If a
// worker with that key does not exist, the VALID flag of the return
// structure is false.
worker_t get_worker(ukey_t key)
{
    worker_t w;
    size_t index;

    assert(key < WORKER_COUNT);

    memset(&w, 0, sizeof(w));

    index = keyfind(w_key, _count, key);
    if (w_key[index] != key) {
        w.valid = false;
    }
    else {
        w.valid = true;
        w.key = key;
        w.name = w_name[index];
        w.zaddr = w_zaddr[index];
        w.io = w_iodir[index];
    }
    return w;
}

key_t name2key(const char name, size_t len);
