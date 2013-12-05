/*
  worker.c - connected peers

  Copyright 2013 Michael L. Gran <spk121@yahoo.com>

  This file is part of Jozabad.

  Jozabad is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Jozabad is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Jozabad.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <glib.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "worker.h"
#include "packet.h"
#include "tput.h"
#include "iodir.h"
#include "seq.h"
#include "cause.h"
#include "diag.h"
#include "channel.h"
#include "packet.h"
#include "poll.h"
#include "msg.h"
#include "joza_msg.h"
#include "lib.h"

#define CALL_REQUEST_DATA_LEN (16)

/*
  These arrays form a key table whose data is keyed either by
  a string key of the ZMQ address or by name.

  wkey  wkey_t   [primary key]    a key created from the ZMQ Router identity
  name  char[17]   [secondary key]  a string name for the connection
  zaddr zframe_t *                  a ZMQ frame containing a ZMQ Router identity
  iodir iodir_t                     whether this worker makes or receives calls
  lcn   lcn_t                       Logical Channel Number
  role  role_t                      X (caller), Y (callee), or READY
  ctime double                    time this worker was created
  mtime double                    time of last message dispatched by this worker

  nidx  worker_idx_t     [secondary key]  an array that keeps the sort indices
  for the secondary key
*/

worker_t *worker_create(void *sock, const zframe_t *A, const char *N, iodir_t io)
{
    worker_t *worker = NULL;
    wkey_t key = 0;
    worker_idx_t i;
    guint64 elapsed_time = g_get_monotonic_time();

    g_message("In %s(A = %p, N = %s, io = %d)", __FUNCTION__, A, N, io);

    key = msg_addr2key(A);
    // First, validate the message
    if (strnlen_s(N, NAME_LEN + 1) == 0) {
        diagnostic(sock, A, c_malformed_message, d_calling_address_too_short);
        g_warning("%s: calling address too short", N);
    } else if (strnlen_s(N, NAME_LEN + 1) > NAME_LEN) {
        diagnostic(sock, A, c_malformed_message, d_calling_address_too_long);
        g_warning("%s: calling address too long", N);
    } else if (!safeascii(N, NAME_LEN)) {
        diagnostic(sock, A, c_malformed_message, d_calling_address_format_invalid);
        g_warning("%s: calling address invalid format", N);
    } else if (!iodir_validate(io)) {
        diagnostic(sock, A, c_malformed_message, d_invalid_directionality_facility);
        g_warning("%s: directionality invalid - %d", io);
    } else {
        worker = g_new0(worker_t, 1);
        worker->wkey = key;
        worker->name = g_strdup(N);
        worker->zaddr = zframe_dup(A);
        worker->iodir = io;
        worker->lcn = 0;
        worker->role = READY;
        worker->ctime = elapsed_time;
        worker->mtime = elapsed_time;
    }
    return worker;
}

// Number of workers
static worker_idx_t _count = 0;

// Data for the workers
wkey_t   w_wkey[WORKER_COUNT];
char     w_name[WORKER_COUNT][NAME_LEN + 1];
zframe_t *w_zaddr[WORKER_COUNT];
iodir_t  w_iodir[WORKER_COUNT];
lcn_t    w_lcn[WORKER_COUNT];
role_t   w_role[WORKER_COUNT];
static double   w_ctime[WORKER_COUNT];
static double   w_mtime[WORKER_COUNT];

// List of the sort order of the strings in w_name
static worker_idx_t   w_nidx[WORKER_COUNT];
// FIXME: PEDANTIC - pointers to the names in w_name;
static char     *w_pname[WORKER_COUNT];

#define INSERT(arr,idx,count)                                           \
    memmove(arr + idx + 1, arr + idx, sizeof(arr[0]) * (count - idx))


static void do_call_request(void *sock, joza_msg_t *M, worker_idx_t I);
static void do_directory_request(void *sock, joza_msg_t *M);
static void do_disconnect(void *sock, joza_msg_t *M);
static worker_idx_t find_idx_from_key(wkey_t arr[], worker_idx_t n, wkey_t X);
static void init_pname(void);

// Initialize a matrix of pointers to a static store.  This whole init_pname
// pedantry is just there to avoid C++ compiler warnings.
static void init_pname(void)
{
    worker_idx_t i;
    for (i = 0; i < WORKER_COUNT; i ++)
        w_pname[i] = &(w_name[i][0]);
}

//----------------------------------------------------------------------------
// SEARCHING ORDERED WKEY_T ARRAYS
//----------------------------------------------------------------------------

// Search a matrix ARR of length N of strictly monotonically increasing
// integers. Return the location of X, or, if X is not found, return the
// location where X would be inserted.
static worker_idx_t find_idx_from_key(wkey_t arr[], worker_idx_t n, wkey_t X)
{
    worker_idx_t lo, hi, mid;

    lo = 0;
    hi = n;
    while (hi - lo > 1) {
        mid = (hi + lo) >> 1;
        if (X >= arr[mid])
            lo = mid;
        else
            hi = mid;
    }
    if (X <= arr[lo])
        return lo;
    return hi;
}

//----------------------------------------------------------------------------
// INDEX SORTING OF STRINGS
//----------------------------------------------------------------------------

// Give an array of strings ARR, and an array of indices that point to
// locations in ARR, and 3 locations into the array of indices, swap
// the three specified index elements so that the strings to which
// they point would be in lexicographic order.
static void sort3 (char *arr[], worker_idx_t indx[], worker_idx_t a, worker_idx_t b, worker_idx_t c)
{
    worker_idx_t tmp;
#define SWAP(a,b) tmp=(a);(a)=(b);(b)=tmp;

    if (strcmp(arr[indx[a]], arr[indx[c]]) > 0) {
        SWAP(indx[a], indx[c]);
    }
    if (strcmp(arr[indx[b]], arr[indx[c]]) > 0) {
        SWAP(indx[b], indx[c]);
    }
    if (strcmp(arr[indx[a]], arr[indx[b]]) > 0) {
        SWAP(indx[a],indx[b]);
    }
#undef SWAP
}

// Using a slow sort, create an index array INDX whose indices would
// sort the values ARR.  Only modify the entries between [left,right).
static void iisort(char *arr[], worker_idx_t indx[], worker_idx_t left, worker_idx_t right)
{
    worker_idx_t i, j;
    char *val;
    worker_idx_t index;

    for (j = left + 1; j < right; j ++) {
        index = indx[j];
        val = arr[index];
        i = j;
        while (i > left && strcmp(arr[indx[i-1]], val) > 0) {
            indx[i] = indx[i-1];
            i --;
        }
        indx[i] = index;
    }
}


// NSTACK is the depth of the stack used to hold Quicksort brackets.
#define NSTACK 50

// M is the size of a bracket where sort switches from Quick Sort to
// Insertion Sort.
static const int M = 7;

// Using a Quicksort, fill an index array INDX whose indices would
// sort the values ARR.  This is adapted from Numerical Recipes in C.
static void name_index_sort(char *arr[], worker_idx_t n, worker_idx_t indx[])
{
    worker_idx_t left = 0;
    worker_idx_t center;
    worker_idx_t right = n - 1;
    worker_idx_t i, j, index_cur, itemp;
    int stack_size = 0, *stack;
    char *value_cur;
#define SWAP(a,b) itemp=(a);(a)=(b);(b)=itemp;

    stack = (int *)calloc(NSTACK, sizeof(int));
    for(j = 0; j < n; j ++)
        indx[j] = j;

    for(;;) {
        // If our subarray is down to a handful of elements, we switch
        // to an insertion sort.
        if (right - left < M) {
            iisort(arr, indx, left, right);
            if (stack_size == 0)
                break;

            // Pop the stack and begin a new round of partitioning.
            right = stack[stack_size-1];
            stack_size --;
            left = stack[stack_size-1];
            stack_size --;
        } else {
            // Choose the center string of left, center, and right
            // elements as partitioning element.  Rearrange the three
            // elements in sorted order.
            center = (left + right) >> 1;
            SWAP(indx[center], indx[left + 1]);
            sort3(arr, indx, left, left + 1, right);

            i = left + 1;
            j = right;

            index_cur = indx[left+1];
            value_cur = arr[index_cur];
            for(;;) {
                // Scan up to find a string greater than target
                // string.
                do {
                    i++;
                } while(strcmp(arr[indx[i]], value_cur) < 0);
                // Scan down to find a string less that our target
                // string.
                do {
                    j--;
                } while(strcmp(arr[indx[j]], value_cur) > 0);
                // If the indices cross, partitioning is complete.
                if(j<i)
                    break;
                // Otherwise, exchange them.
                SWAP(indx[i],indx[j]);
            }
            indx[left+1] = indx[j];
            // Insert the index of the target string here.
            indx[j] = index_cur;
            stack_size += 2;
            // Push pointers to a larger subarray on the stack, and
            // process smaller subarray immediately.
            if(stack_size>NSTACK) abort();
            if (right - i + 1 >= j - left) {
                stack[stack_size-1] = right;
                stack[stack_size-2] = i;
                right = j - 1;
            } else {
                stack[stack_size-1] = j - 1;
                stack[stack_size-2] = left;
                left = i;
            }
        }
    }
    free (stack);
#undef SWAP
}



// Add a new worker to the store.  Returns the key of the new worker, or
// zero on failure.
wkey_t worker_add(void *sock, const zframe_t *A, const char *N, iodir_t io)
{
    wkey_t key = 0;
    worker_idx_t i;
    double elapsed_time = now();

    assert(_count < WORKER_COUNT);

    g_message("In %s(A = %p, N = %s, io = %d)", __FUNCTION__, A, N, io);

    key = msg_addr2key(A);
    i = find_idx_from_key(w_wkey, _count, key);
    assert (w_wkey[i] != key);

    // First, validate the message
    if (strnlen_s(N, NAME_LEN + 1) == 0) {
        diagnostic(sock, A, c_malformed_message, d_calling_address_too_short);
        g_warning("%s: calling address too short", N);
    } else if (strnlen_s(N, NAME_LEN + 1) > NAME_LEN) {
        diagnostic(sock, A, c_malformed_message, d_calling_address_too_long);
        g_warning("%s: calling address too long", N);
    } else if (!safeascii(N, NAME_LEN)) {
        diagnostic(sock, A, c_malformed_message, d_calling_address_format_invalid);
        g_warning("%s: calling address invalid format", N);
    } else if (!iodir_validate(io)) {
        diagnostic(sock, A, c_malformed_message, d_invalid_directionality_facility);
        g_warning("%s: directionality invalid - %d", io);
    } else if (_count >= WORKER_COUNT) {
        // FIXME: culling of old connections would go here.
        diagnostic(sock, A, c_network_congestion, d_no_connections_available);
        g_warning("%s: cannot add. No free connections", N);
    } else {
        if (i < _count) {
            INSERT(w_wkey, i, _count);
            INSERT(w_zaddr, i, _count);
            INSERT(w_name, i, _count);
            INSERT(w_iodir, i, _count);
            INSERT(w_lcn, i, _count);
            INSERT(w_role, i, _count);
            INSERT(w_ctime, i, _count);
            INSERT(w_mtime, i, _count);
        }

        w_wkey[i] = key;
        memset(w_name[i], 0, NAME_LEN + 1);
        strncpy(w_name[i], N, NAME_LEN);
        w_zaddr[i] = zframe_dup((zframe_t *)A);
        w_iodir[i] = io;
        w_lcn[i] = LCN_C(0);
        w_role[i] = READY;

        w_ctime[i] = elapsed_time;
        w_mtime[i] = elapsed_time;
        _count ++;

        // Update the index table that alphabetizes the names.
        init_pname();
        name_index_sort(w_pname, _count, w_nidx);
        g_message("added new channel %s, %s at index %d", N, iodir_name(io), i);
        joza_msg_send_addr_connect_indication(sock, A);
    }
    g_message("%s(A = %p, N = %s, io = %d) returns %d", __FUNCTION__, A, N, io, key);

    return key;
}

bool_index_t worker_get_idx_by_key(wkey_t key)
{
    bool_index_t bi;
    bi.flag = FALSE;
    bi.index = 0;

    if (key != 0) {
        bi.index = find_idx_from_key(w_wkey, _count, key);
        if (w_wkey[bi.index] == key)
            bi.flag = TRUE;
    }
    return bi;
}

// Using an unordered matrix of stringx ARR of length N, an an index
// matrix whose contents order the string matrix return the location
// of the STR, or, if it is not found, the last location checked.  A
// return value of N indicates after the end of the matrix.
static worker_idx_t find_name(const char *str)
{
    worker_idx_t i = 0;
    while (i < _count) {
        if (strcmp(str, w_name[i]) == 0)
            return i;
        i++;
    }
    return _count;
#if 0
    worker_idx_t lo, hi, mid;
    // FIXME: this bisect search should be made to work
    lo = 0;
    hi = _count;
    while (hi - lo > 1) {
        mid = (hi + lo) >> 1;
        if (strcmp(str, w_name[w_nidx[mid-1]]) >= 0)
            lo = mid;
        else
            hi = mid;
    }
    return lo;
#endif
}

static bool_index_t worker_get_idx_by_name(const char *name)
{
    bool_index_t bi;
    bi.flag = FALSE;
    bi.index = 0;

    if (name != 0) {
        bi.index = find_name(name);
        if (bi.index < _count && strcmp(w_name[bi.index], name) == 0)
            bi.flag = TRUE;
    }
    return bi;
}

#define REMOVE(arr,idx,count)                                           \
    do {                                                                \
        memmove(arr + idx, arr + idx + 1, sizeof(arr[0]) * (count - idx - 1)); \
        memset(arr + count - 1, 0, sizeof(arr[0]));                     \
    }                                                                   \
    while(0)

void remove_worker_by_key(wkey_t key)
{
    remove_worker(key);
}

void remove_worker(wkey_t key)
{
    worker_idx_t i;
    if (_count == 0)
        return;
    if (key == 0)
        return;
    i = find_idx_from_key(w_wkey, _count, key);
    if (w_wkey[i] != key)
        return;
    zframe_destroy(&w_zaddr[i]);
    if (i != WORKER_IDX_MAX && i < _count - 1) {
        REMOVE(w_wkey, i, _count);
        REMOVE(w_zaddr, i, _count);
        REMOVE(w_name, i, _count);
        REMOVE(w_iodir, i, _count);
        REMOVE(w_lcn, i, _count);
        REMOVE(w_role, i, _count);
        REMOVE(w_ctime, i, _count);
        REMOVE(w_mtime, i, _count);
    }
    _count --;
}

#if 0
gboolean worker_dispatch_by_idx (void *sock, joza_msg_t *M, worker_idx_t I)
{
    gboolean more = FALSE;        /* When TRUE, message needs dowmstream processing.  */
    const char *cmdname, *name;
    int id;

    cmdname = joza_msg_const_command(M);
    id = joza_msg_id(M);
    name = w_name[I];
    w_mtime[I] = now();

    g_message("handling %s from connected worker '%s'", cmdname, name);
    switch(id) {
    case JOZA_MSG_CALL_REQUEST:
        // Worker has requested a channel to another worker
        do_call_request(sock, M, I);
        break;
    case JOZA_MSG_DIRECTORY_REQUEST:
        // Worker has request a list of the currently connected workers
        do_directory_request(sock, M);
        break;
    case JOZA_MSG_DISCONNECT:
        // Worker has requested to be disconnected
        do_disconnect(sock, M);
        break;
    default:
        break;
    }
    return more;
}

static void do_call_request(void *sock, joza_msg_t *M, worker_idx_t I)
{
    zframe_t *addr       = joza_msg_address(M);
    char     *xname      = joza_msg_calling_address(M);
    char     *yname      = joza_msg_called_address(M);
    packet_t pkt         = (packet_t) joza_msg_packet(M);
    int      pkt_rcheck  = packet_rngchk(pkt);
    tput_t   tput        = (tput_t) joza_msg_throughput(M);
    int      tput_rcheck = tput_rngchk(tput);
    seq_t    window      = joza_msg_window(M);
    int      window_rcheck = window_rngchk(window);
    size_t   data_len    = zframe_size(joza_msg_data(M));
    bool_index_t bi_y    = worker_get_idx_by_name(yname);

    // Validate the message

    if (strnlen_s(xname, NAME_LEN + 1) == 0)
        diagnostic(sock, addr, c_malformed_message, d_calling_address_too_short);
    else if (strnlen_s(xname, NAME_LEN + 1) > NAME_LEN)
        diagnostic(sock, addr, c_malformed_message, d_calling_address_too_long);
    else if (!safeascii(xname, NAME_LEN))
        diagnostic(sock, addr, c_malformed_message, d_calling_address_format_invalid);
    else if (strnlen_s(yname, NAME_LEN + 1) == 0)
        diagnostic(sock, addr, c_malformed_message, d_called_address_too_short);
    else if (strnlen_s(yname, NAME_LEN + 1) > NAME_LEN)
        diagnostic(sock, addr, c_malformed_message, d_called_address_too_long);
    else if (!safeascii(yname, NAME_LEN))
        diagnostic(sock, addr, c_malformed_message, d_called_address_format_invalid);
    else if (pkt_rcheck < 0)
        diagnostic(sock, addr, c_malformed_message, d_packet_facility_too_small);
    else if (pkt_rcheck > 0)
        diagnostic(sock, addr, c_malformed_message, d_packet_facility_too_large);
    else if (tput_rcheck < 0)
        diagnostic(sock, addr, c_malformed_message, d_throughput_facility_too_small);
    else if (tput_rcheck > 0)
        diagnostic(sock, addr, c_malformed_message, d_throughput_facility_too_large);
    else if (window_rcheck < 0)
        diagnostic(sock, addr, c_malformed_message, d_window_facility_too_small);
    else if (window_rcheck > 0)
        diagnostic(sock, addr, c_malformed_message, d_window_facility_too_large);
    else if (data_len > CALL_REQUEST_DATA_LEN)
        diagnostic(sock, addr, c_malformed_message, d_data_too_long);

    // Check that the other peer is ready to receive a call

    else if (bi_y.flag == FALSE)
        diagnostic(sock, addr, c_unknown_address, d_unknown_worker_address);
    else if (w_role[bi_y.index] != READY)
        diagnostic(sock, addr, c_number_busy, d_number_busy);
    else if (w_iodir[bi_y.index] == io_incoming_calls_barred)
        diagnostic(sock, addr, c_access_barred, d_input_barred);

    // Check that there is a free channel to the message

    else if (channel_available() == FALSE)
        // FIXME: logic to destroy unused channels goes here
        diagnostic(sock, addr, c_network_congestion, d_no_channels_available);

    else {

        // Throttle the facilities to this broker's maxima.
        //pkt    = packet_throttle(pkt, opt_packet);
        //window = window_throttle(window, opt_window);
        //tput   = tput_throttle(tput, opt_tput);

        // Finally, create the raw channel
        lcn_t lcn = channel_add(w_zaddr[I], w_pname[I], w_zaddr[bi_y.index], w_pname[bi_y.index], pkt, window, tput);
        w_lcn[I] = lcn;
        w_lcn[bi_y.index] = lcn;
        w_role[I] = X_CALLER;
        w_role[bi_y.index] = Y_CALLEE;

        // FIXME: set the call request timer

        // Send the call request to the peer
        joza_msg_send_addr_call_request(sock, w_zaddr[bi_y.index],
                                        xname, yname, pkt, window, tput, joza_msg_data(M));
    }
}

static void do_directory_request(void *sock, joza_msg_t *M)
{
    zframe_t *addr       = joza_msg_address(M);
    zhash_t  *dir       = zhash_new();

    // Fill a hash table with the current directory information
    // FIXME: probably could put something useful in the "value" part of the
    // hash table.
    for (worker_idx_t i = 0; i < _count; i ++) {
        zhash_insert(dir, w_pname[i], "");
    }
    directory_request(sock, addr, dir);
    zhash_destroy(&dir);
}


// Disconnect this channel, which is not currently on a call.
static void do_disconnect(void *sock, joza_msg_t *M)
{
    const zframe_t *addr = joza_msg_const_address(M);
    wkey_t key = msg_addr2key(addr);
    joza_msg_send_addr_disconnect_indication(sock, addr);
    remove_worker (key);
}

void worker_remove_all()
{
    for (int i = _count - 1; i >= 0; i --) {
        zframe_destroy(&w_zaddr[i]);
        REMOVE(w_wkey, i, _count);
        REMOVE(w_zaddr, i, _count);
        REMOVE(w_name, i, _count);
        REMOVE(w_iodir, i, _count);
        REMOVE(w_lcn, i, _count);
        REMOVE(w_role, i, _count);
        REMOVE(w_ctime, i, _count);
        REMOVE(w_mtime, i, _count);
    }
}

zhash_t *worker_directory()
{
    zhash_t *dir = zhash_new();
    for (worker_idx_t i = 0; i < _count; i ++) {
        zhash_insert(dir, w_pname[i], "");
    }
    return dir;
}
#endif
