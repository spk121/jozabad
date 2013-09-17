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

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"
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
#include "log.h"
#include "msg.h"
#include "joza_msg.h"

#define CALL_REQUEST_DATA_LEN (16)
extern int PARANOIA = 0;

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
  mtime uint64_t                    time of last message dispatched by this worker

  nidx  size_t     [secondary key]  an array that keeps the sort indices
                                    for the secondary key
*/

// Number of workers
static size_t   _count = 0;

// Data for the workers
uint32_t w_zhash[WORKER_COUNT];
char     w_name[WORKER_COUNT][NAME_LEN + 1];
zframe_t *w_zaddr[WORKER_COUNT];
iodir_t  w_iodir[WORKER_COUNT];
ukey_t   w_lcn[WORKER_COUNT];
role_t   w_role[WORKER_COUNT];
static double   w_ctime[WORKER_COUNT];
static double   w_mtime[WORKER_COUNT];

// List of the sort order of the strings in w_name
static size_t   w_nidx[WORKER_COUNT];
// FIXME: PEDANTIC - pointers to the names in w_name;
static char     *w_pname[WORKER_COUNT];

static void do_call_request(joza_msg_t *M, size_t I);
static void do_disconnect(joza_msg_t *M, size_t I);


void init_pname(void)
{
    for (size_t i = 0; i < WORKER_COUNT; i ++)
        w_pname[i] = &(w_name[i][0]);
}

#define INSERT(arr,idx,count)                               \
    memmove(arr + idx + 1, arr + idx, sizeof(arr[0]) * (count - idx))

static void pushd(double arr[], size_t idx, size_t count)
{
    if (PARANOIA) {
        for (size_t i = _count; i >= idx + 1; i --) {
            arr[i] = arr[i-1];
        }
        arr[idx] = 0.0;
    } else
        memmove(&arr[idx+1], &arr[idx], sizeof(double) * (count - idx));
}

// Add a new worker to the store.  Returns the hash key of the new
// worker, or zero on failure.
uint32_t add_worker(const zframe_t *A, const char *N, iodir_t I)
{
    uint32_t hash;
    size_t i;
    double elapsed_time = now();

    if (_count >= WORKER_COUNT)
        return 0;

    hash = msg_addr2hash(A);
    if (hash == 0)
        return 0;

    i = ifind(w_zhash, _count, hash);
    if (w_zhash[i] == hash)
        return 0;

    if (strnlen_s(N, NAME_LEN + 1) > NAME_LEN
            || !safeascii(N, NAME_LEN))
        return 0;
    if (!iodir_validate(I))
        return 0;

    if (i < _count) {
        INSERT(w_zhash, i, _count);
        INSERT(w_zaddr, i, _count);
        INSERT(w_name, i, _count);
        INSERT(w_iodir, i, _count);
        INSERT(w_lcn, i, _count);
        INSERT(w_role, i, _count);
        INSERT(w_ctime, i, _count);
        INSERT(w_mtime, i, _count);
    }

    w_zhash[i] = hash;
    memset(w_name[i], 0, NAME_LEN + 1);
    strncpy(w_name[i], N, NAME_LEN);
    w_zaddr[i] = zframe_dup((zframe_t *)A);
    w_iodir[i] = I;
    w_lcn[i] = UKEY_C(0);
    w_role[i] = READY;

    w_ctime[i] = elapsed_time;
    w_mtime[i] = elapsed_time;

    // Update the index table that alphabetizes the names.
    qisort(w_pname, _count, w_nidx);
    return hash;
}

bool_index_t worker_get_idx_by_hash(uint32_t hash)
{
    bool_index_t bi;
    bi.flag = FALSE;
    bi.index = 0;

    if (hash != 0) {
        bi.index = ifind(w_zhash, _count, hash);
        if (w_zhash[bi.index] == hash)
            bi.flag = TRUE;
    }
    return bi;
}

// Using an unordered matrix of stringx ARR of length N, an an index
// matrix whose contents order the string matrix return the location
// of the STR, or, if it is not found, the last location checked.  A
// return value of N indicates after the end of the matrix.
static size_t find_name(const char *str)
{
    size_t lo, hi, mid;

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
}

bool_index_t worker_get_idx_by_name(const char *name)
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

void remove_worker_by_hash(uint32_t hash)
{
    remove_worker(hash);
}

void remove_worker(uint32_t hash)
{
    size_t i;
    if (_count == 0)
        return;
    if (hash == 0)
        return;
    i = ifind(w_zhash, _count, hash);
    if (w_zhash[i] != hash)
        return;
    if (i < _count - 1) {
        REMOVE(w_zhash, i, _count);
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

#undef REMOVE


bool_t worker_dispatch_by_idx (joza_msg_t *M, size_t I)
{
    bool_t more = FALSE;        /* When TRUE, message needs dowmstream processing.  */
    const char *cmdname, *name;
    int id;

    cmdname = joza_msg_const_command(M);
    id = joza_msg_id(M);
    name = w_name[I];
    w_mtime[I] = now();

    INFO("handling %s from connected worker '%s'", cmdname, name);
    switch(id) {
    case JOZA_MSG_CALL_REQUEST:
        // Worker has requested a channel to another worker
        do_call_request(M, I);
        break;
    case JOZA_MSG_DISCONNECT:
        // Worker has requested to be disconnected
        do_disconnect(M, I);
        break;
    default:
        break;
    }
    return more;
}

static void do_call_request(joza_msg_t *M, size_t I)
{
    zframe_t *addr       = joza_msg_address(M);
    char     *xname      = joza_msg_calling_address(M);
    char     *yname      = joza_msg_called_address(M);
    packet_t pkt         = (packet_t) joza_msg_packet(M);
    int      pkt_rcheck  = packet_rngchk(pkt);
    tput_t   tput        = (tput_t) joza_msg_throughput(M);
    int      tput_rcheck = tput_rngchk(tput);
    seq_t    window      = joza_msg_window(M);
    int      window_rcheck = seq_rngchk(window);
    uint8_t  *data       = zframe_data(joza_msg_data(M));
    size_t   data_len    = zframe_size(joza_msg_data(M));
    bool_index_t bi_y    = worker_get_idx_by_name(yname);

    // Validate the message

    if (strnlen(xname, NAME_LEN + 1) == 0)
        DIAGNOSTIC(addr, c_malformed_message, d_calling_address_too_short);
    else if (strnlen(xname, NAME_LEN + 1) > NAME_LEN)
        DIAGNOSTIC(addr, c_malformed_message, d_calling_address_too_long);
    else if (!safeascii(xname, NAME_LEN))
        DIAGNOSTIC(addr, c_malformed_message, d_calling_address_format_invalid);
    else if (strnlen(yname, NAME_LEN + 1) == 0)
        DIAGNOSTIC(addr, c_malformed_message, d_called_address_too_short);
    else if (strnlen(yname, NAME_LEN + 1) > NAME_LEN)
        DIAGNOSTIC(addr, c_malformed_message, d_called_address_too_long);
    else if (!safeascii(yname, NAME_LEN))
        DIAGNOSTIC(addr, c_malformed_message, d_called_address_format_invalid);
    else if (pkt_rcheck < 0)
        DIAGNOSTIC(addr, c_malformed_message, d_packet_facility_too_small);
    else if (pkt_rcheck > 0)
        DIAGNOSTIC(addr, c_malformed_message, d_packet_facility_too_large);
    else if (tput_rcheck < 0)
        DIAGNOSTIC(addr, c_malformed_message, d_throughput_facility_too_small);
    else if (tput_rcheck > 0)
        DIAGNOSTIC(addr, c_malformed_message, d_throughput_facility_too_large);
    else if (window_rcheck < 0)
        DIAGNOSTIC(addr, c_malformed_message, d_window_facility_too_small);
    else if (window_rcheck > 0)
        DIAGNOSTIC(addr, c_malformed_message, d_window_facility_too_large);
    else if (data_len > CALL_REQUEST_DATA_LEN)
        DIAGNOSTIC(addr, c_malformed_message, d_data_too_long);

    // Check that the other peer is ready to receive a call

    else if (bi_y.flag == FALSE)
        DIAGNOSTIC(addr, c_unknown_address, d_unknown_worker_address);
    else if (w_role[bi_y.index] != READY)
        DIAGNOSTIC(addr, c_number_busy, d_number_busy);
    else if (w_iodir[bi_y.index] == io_incoming_calls_barred)
        DIAGNOSTIC(addr, c_access_barred, d_input_barred);

    // Check that there is a free channel to the message

    else if (channel_available() == FALSE)
        // FIXME: logic to destroy unused channels goes here
        DIAGNOSTIC(addr, c_network_congestion, d_no_channels_available);

    else {

        // Finally, create the raw channel
        ukey_t lcn = channel_add(addr, xname, w_zaddr[bi_y.index], yname);
        w_lcn[I] = lcn;
        w_lcn[bi_y.index] = lcn;
        w_role[I] = X_CALLER;
        w_role[bi_y.index] = Y_CALLEE;

        // Throttle the facilities to this broker's maxima.
        //pkt    = packet_throttle(pkt, opt_packet);
        //window = window_throttle(window, opt_window);
        //tput   = tput_throttle(tput, opt_tput);

        // Set the initial facilities values
        size_t idx = ukey_find(c_lcn, _count, lcn);
        c_pkt[idx] = pkt;
        c_tput[idx] = tput;
        c_window[idx] = window;

        // Transition state to X_CALL
        c_state[idx] = state_x_call_request;

        // FIXME: set the call request timer

        // Send the call request to the peer
        joza_msg_send_addr_call_request (g_poll_sock, w_zaddr[bi_y.index], xname, yname, pkt, window, tput, zframe_dup(joza_msg_data(M)));
    }
}

// Disconnect this channel, which is not currently on a call.
static void do_disconnect(joza_msg_t *M, size_t I)
{
    const zframe_t *addr = joza_msg_const_address(M);
    uint32_t hash = msg_addr2hash(addr);
    joza_msg_send_addr_disconnect_indication(g_poll_sock, addr);
    remove_worker (hash);
}
