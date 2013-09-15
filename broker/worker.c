#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"
#include "worker.h"
#include "iodir.h"
#include "../libjoza/joza_msg.h"

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
void     *w_zaddr[WORKER_COUNT];
iodir_t  w_iodir[WORKER_COUNT];
ukey_t   w_lcn[WORKER_COUNT];
role_t   w_role[WORKER_COUNT];
static double   w_ctime[WORKER_COUNT];
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
uint32_t addr2hash (zframe_t *z)
{
    uint32_t x[1];
#ifdef HAVE_CZMQ
    memcpy(x, (char *) zframe_data(z) + 1, sizeof(uint32_t));
#else
	// Unsafe dependency on data format knowledge!!
	// Skip two byte frame header and the initial zero
	// that the router socket places in a router address frame.
	memcpy(x, (char *) z + 3, sizeof(uint32_t));
#endif
    return x[0];
}

#define PUSH(arr,idx,count)                               \
    memmove(arr + idx + 1, arr + idx, sizeof(arr[0]) * (count - idx))

static void pushd(double arr[], size_t idx, size_t count)
{
    if (PARANOIA)
    {
        for (size_t i = _count; i >= idx + 1; i --)
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
        return 0;

    hash = addr2hash(A);
    if (hash == 0)
        return 0;

    i = ifind(w_zhash, _count, hash);
    if (w_zhash[i] == hash)
        return 0;

    if (strnlen_s(N, NAME_LEN + 1) > NAME_LEN
        || !safeascii(N, NAME_LEN))
        return 0;
    if (!VAL_IODIR(I))
        return 0;
    
    if (i < _count) {
        PUSH(w_zhash, i, _count);
        PUSH(w_zaddr, i, _count);
        PUSH(w_name, i, _count);
        PUSH(w_iodir, i, _count);
        PUSH(w_lcn, i, _count);
        PUSH(w_role, i, _count);
        pushd(w_ctime, i, _count);
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

    // Update the index table that alphabetizes the names.
    qisort(w_pname, _count, w_nidx);
    return hash;
}

bool_index_t get_worker(uint32_t hash)
{
    bool_index_t bi;
    bi.flag = FALSE;
    bi.index = 0;

    if (hash != 0)
    {
        bi.index = ifind(w_zhash, _count, hash);
        if (w_zhash[bi.index] == hash)
            bi.flag = TRUE;
    }
    return bi;
}

void remove_worker(uint32_t hash)
{
	size_t i;
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
        removed(w_mtime, i, _count);
    }
    _count --;
    w_zhash[_count] = 0;
    memset(w_name[_count], 0, NAME_LEN + 1);
    w_zaddr[_count] = NULL;
    w_iodir[i] = FREE;
    w_lcn[i] = UKEY_C(0);
    w_role[i] = READY;
    w_ctime[i] = -1.0;
    w_mtime[i] = -1.0;
}

#undef REMOVE


bool_t worker_dispatch_by_idx (joza_msg_t *M, size_t I)
{
    bool_t more = FALSE;        /* When TRUE, message needs dowmstream processing.  */
    const char *cmdname, *name;
    int id;

    cmdname = joza_msg_const_address(M);
    id = joza_msg_id(M);
    name = w_name[I];
    w_mtime[I] = now();

    INFO("handling %s from connected worker '%s'", cmdname, name);
    switch(id) {
    case JOZA_MSG_CALL_REQUEST:
        // Worker has requested a channel to another worker
        do_call_request();
        break;
    case JOZA_MSG_DISCONNECT:
        // Worker has requested to be disconnected
        do_disconnect();
        break;
    default:
        break;
    }
    return more;
}
    
#define DSELF(X) do {joza_msg_send_diagnostic(g_sock, addr, SELF, (X))} while (0)
#define DBROKER(X) do {joza_msg_send_diagnostic(g_sock, addr, BROKER, (X))} while (0)
#define DOTHER(X) do {joza_msg_send_diagnostic(g_sock, addr, OTHER, (X))} while (0)

void do_call_request(joza_msg_t *M, size_t I)
{
    zframe_t *addr       = joza_msg_address(M);
    char     *xname      = joza_msg_calling_address(M);
    char     *yname      = joza_msg_called_address(M);
    packet_t pkt         = (packet_t) joza_msg_packet(M);
    int      pkt_rcheck  = rngchk_packet(pkt);
    tput_t   tput        = (tput_t) joza_msg_throughput(M);
    int      tput_rcheck = rngchk_tput(tput);
    seq_t    window      = joza_msg_window(M);
    int      window_rcheck = rngchk_window(window);
    uint8_t  *data       = zframe_data(joza_msg_data(M));
    size_t   data_len    = zframe_size(joza_msg_data(M));
    bool_index_t bi_y    = get_worker_by_name(yname);

    // Validate the message

    if (strnlen(xname, NAME_LEN + 1) == 0)
        DSELF(D_CALLING_ADDRESS_TOO_SHORT);
    else if (strnlen(xname, NAME_LEN + 1) > NAME_LEN)
        DSELF(D_CALLING_ADDRESS_TOO_LONG);
    else if (!safeascii(xname))
        DSELF(D_CALLING_ADDRESS_INVALID);
    else if (strnlen(yname, NAME_LEN + 1) == 0)
        DSELF(D_CALLED_ADDRESS_TOO_SHORT);
    else if (strnlen(yname, NAME_LEN + 1) > NAME_LEN)
        DSELF(D_CALLED_ADDRESS_TOO_LONG);
    else if (!safeascii(yname))
        DSELF(D_CALLED_ADDRESS_INVALID);
    else if (pkt_rcheck < 0)
        DSELF(D_PACKET_FACILITY_TOO_SMALL);
    else if (pkt_rcheck > 0)
        DSELF(D_PACKET_FACILITY_TOO_LARGE);
    else if (tput_rcheck < 0)
        DSELF(D_THROUGHPUT_FACILITY_TOO_SMALL);
    else if (tput_rcheck > 0)
        DSELF(D_THROUGHPUT_FACILITY_TOO_LARGE);
    else if (window_rcheck < 0)
        DSELF(D_WINDOW_FACILITY_TOO_SMALL);
    else if (window_rcheck > 0)
        DSELF(D_WINDOW_FACILITY_TOO_LARGE);
    else if (data_len > CALL_REQUEST_DATA_LEN)
        DSELF(D_DATA_TOO_LARGE);

    // Check that the other peer is ready to receive a call

    else if (bi_y.flag == FALSE)
        DBROKER(D_CALLEE_UNKNOWN_ADDRESS);
    else if (w_role[bi_y.index] != READY)
        DBROKER(D_CALLEE_BUSY);
    else if (w_iodir[bi_y.index] == OUTPUT)
        DBROKER(D_CALLEE_INCOMING_CALLS_BARRED);

    // Check that there is a free channel to the message

    else if (channel_available() == FALSE)
        // FIXME: logic to destroy unused channels goes here
        DBROKER(D_NO_CHANNELS_AVAILABLE);
        
    else {

        // Finally, create the raw channel
        ukey_t lcn = add_channel(addr, xname, w_addr[bi_y.index], yname);
        w_lcn[I] = lcn;
        w_lcn[bi_y.index] = lcn;
        w_role[I] = X_CALLER;
        w_role[bi_y.index] = Y_CALLEE;
        
        // Throttle the facilities to this broker's maxima.
        pkt    = packet_throttle(pkt, opt_packet);
        window = window_throttle(window, opt_window);
        tput   = tput_throttle(tput, opt_tput);

        // Set the initial facilities values
        size_t idx = keyfind(c_lcn, _count, lcn);
        c_pkt[idx] = pkg;
        c_tput[idx] = tput;
        c_window[idx] = window;

        // Transition state to X_CALL
        c_state[idx] = state_x_call;

        // FIXME: set the call request timer

        // Send the call request to the peer
        joza_msg_send_call_request (g_sock, addr, xname, yname, pkt, window, tput, zframe_dup(joza_msg_data(msg)));
    }
}

// Disconnect this channel, which is not currently on a call.
void do_disconnect(joza_msg_t *M, size_t I)
{
    zframe_t *addr = joza_msg_addr(m);
    uint32_t hash = addr2hash(addr);
    joza_msg_send_addr_disconnect_indication(g_sock, addr);
    remove_worker (hash);
}
