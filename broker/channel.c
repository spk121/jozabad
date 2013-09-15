#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include "lib.h"
#include "channel.h"
#include "state.h"
#include "tput.h"
#include <czmq.h>
#include "../libjoza/joza_msg.h"
#include "worker.h"


/*
lcn    ukey_t       [primary key]   the unique id for this connection
xzaddr zframe_t *                  a ZMQ frame containing a ZMQ Router identity for X caller
yzaddr zframe_t *                  a ZMQ frame containing a ZMQ Router identity for Y callee
state  state_t                     call status
xps    seq_t                       ID of next packet to be sent by X
xpr    seq_t                       Smallest packet ID X will accept from Y
yps    seq_t                       ID of next packet to be sent by Y
ypr    seq_t                       Smallest packet ID Y will accept from X
window seq_t                       delta between the smallest and largest acceptable ID
tput   tput_t                      throughput allowed on this channel
tok   double                       store for x throughput "leaky bucket" tokens
ctime double                       time this channel was created
mtime double                       time of last message from either peer
*/                                                                


static size_t _count = 0;
static ukey_t _lcn = UKEY_MIN;
static c_lcn[CHANNEL_COUNT];
void *c_xzaddr[CHANNEL_COUNT]; /* ZMQ address of caller X */
void *c_yzaddr[CHANNEL_COUNT]; /* ZMQ address of callee Y */
size_t c_yidx[CHANNEL_COUNT]; /* index array that sorts ykey array */
char *c_xname[CHANNEL_COUNT];
char *c_yname[CHANNEL_COUNT];
static state_t c_state[CHANNEL_COUNT];
static seq_t c_xps[CHANNEL_COUNT]; /* sequence number of packets sent by X */
static seq_t c_xpr[CHANNEL_COUNT]; /* lowest packet sequence permitted to send to X */
static seq_t c_yps[CHANNEL_COUNT]; /* sequence number of packets sent by Y  */
static seq_t c_ypr[CHANNEL_COUNT]; /* lowest packet sequence permitted to send to Y */
static seq_t c_window[CHANNEL_COUNT];
static tput_t c_tput[CHANNEL_COUNT]; /* bits/sec permitted on this channel */
static double c_ctime[CHANNEL_COUNT]; /* time this channel was activated */
static double c_mtime[CHANNEL_COUNT]; /* timestamp of last messate dispatched */

#define PUSH(arr,idx,count)                                             \
    memmove(arr + idx + 1, arr + idx, sizeof(arr[0]) * (count - idx))

bool_t channel_available()
{
    if (_count < CHANNEL_COUNT)
        return TRUE;
    return FALSE;
}

ukey_t add_channel(zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname)
{
    assert(_count < CHANNEL_COUNT);
    assert(xzaddr);
    assert(yzaddr);

    size_t idx;

    idx = keyfind(c_lcn, _count, _lcn);
    assert (c_lcn[idx] != _lcn);

    if (idx < _count)
    {
        PUSH(c_xzaddr, idx, _count);
        PUSH(c_yzaddr, idx, _count);
        PUSH(c_xname, idx, _count);
        PUSH(c_yname, idx, _count);
        PUSH(c_state, idx, _count);
        PUSH(c_xps, idx, _count);
        PUSH(c_yps, idx, _count);
        PUSH(c_xpr, idx, _count);
        PUSH(c_ypr, idx, _count);
        PUSH(c_window, idx, _count);
        PUSH(c_tput, idx, _count);
    }
    c_xzaddr[idx] = xzaddr;
    c_yzaddr[idx] = yzaddr;
    c_xname[idx] = xname;
    c_yname[idx] = yname;
    c_state[idx] = state_ready;
    c_xps[idx] = SEQ_MIN;
    c_xps[idx] = SEQ_MIN;
    c_ypr[idx] = SEQ_MIN;
    c_ypr[idx] = SEQ_MIN;
    c_window[idx] = WINDOW_DEFAULT;
    c_tput[idx] = t_default;

    _count ++;
};    

/* Handle message send a worker on this connected channel */
void channel_dispatch_by_lcn(joza_msg_t *M, ukey_t LCN, role_t R)
{
    const char *cmdname = NULL, *xname = NULL, *yname = NULL;
    bool_index_t bi;
    size_t I;
    state_t state_orig;
    
    cmdname = joza_msg_const_address(M);
    I = ifind(c_lcn, _count, LCN);
    assert (c_lcn[I] == LCN);

    c_mtime[I] = now();

    xname = c_xname[I];
    yname = c_yname[I];
    state_orig = c_state[I];

    if (R == X_CALLER)
        INFO("%s/%s handling %s from %s", xname, yname, cmdname, xname);
    else if (R == Y_CALLEE)
        INFO("%s/%s handling %s from %s", xname, yname, cmdname, yname);
    else
        abort();

    /* Big fat dispatch table */
	a = find_action(c_state[i], joza_msg_const_id(msg), R == Y_CALLEE ? 1 : 0);
	INFO("%s/%s dispatching %s in %s", xname, yname, action_name(a), state_name(state));

	switch (a) {
	case a_unspecified:
	case a_disconnect:
	case a_x_connect:
    case a_x_call_request:
	case a_x_call_accepted:
	case a_x_call_collision:
	case a_y_connect:
	case a_y_call_request:
		// None of these should ever happen.  X is always connected
        // before Y. And this channel is already set-up by the time
        // this function is called.
		abort();
		break;

	case a_discard:
		break;

	case a_reset:
        do_reset(M, I, R);
		break;
	case a_clear: 
        do_clear(M, I);
		break;

	case a_x_disconnect:
        do_x_disconnect(M, I);
		break;
	case a_x_clear_request:
        do_x_clear_request(M, I);
		break;
	case a_x_clear_confirmation:
		do_x_clear_confirmation(M, I);
		break;
	case a_x_data:
        do_x_data(M, I);
		break;
	case a_x_rr:
        do_x_rr(M, I);
		break;
	case a_x_rnr:
		do_x_rnr(M, I);
		break;
	case a_x_reset:
		do_x_reset(M, I);
		break;
	case a_x_reset_confirmation:
		do_x_reset_confirmation(M, I);
		break;

	case a_y_disconnect:
		do_y_disconnect(M, I);
		break;
	case a_y_call_accepted:
		do_y_call_accepted(M, I);
		break;
	case a_y_call_collision:
		do_y_call_collision(M, I);
		break;
	case a_y_clear_request:
		do_y_clear_request(M, I);
		break;
	case a_y_clear_confirmation:
		do_y_clear_confirmation(M, I);
		break;
	case a_y_data:
		do_y_data(M, I);
		break;
	case a_y_rr:
		do_y_rr(M, I);
		break;
	case a_y_rnr:
		do_y_rnr(M, I);
		break;
	case a_y_reset:
		do_y_reset(M, I);
		break;
	case a_y_reset_confirmation:
		do_y_reset_confirmation(M, I);
		break;
	}
}

// This punishment action is a result of a message received a worker
// that is incorrect for the current state.
void do_reset(joza_msg_t *M, int I, role_t R)
{
    int id;
    int state = c_state[I];

    if (R == X_CALLER) {
        joza_msg_send_addr_reset_request (g_sock, c_xaddr, SELF, D_INVALID(id, state));
        joza_msg_send_addr_reset_request (g_sock, c_yaddr, OTHER, D_INVALID(id, state));
    }
    else {
        joza_msg_send_addr_reset_request (g_sock, c_xaddr, OTHER, D_INVALID(id, state));
        joza_msg_send_addr_reset_request (g_sock, c_yaddr, SELF, D_INVALID(id, state));
    }
    reset_channel_flow_by_idx[I];
    c_state[I] = state_y_reset_request;
}

// This punishement action is a result of a message received from a
// worker that is incorrect for the current state.
void do_clear(joza_msg_t *M, int I, role R)
{
    int id;
    int state = c_state[I];

    if (R == X_CALLER) {
        joza_msg_send_addr_clear_request (g_sock, c_xaddr[I], SELF, D_INVALID(id, state));
        joza_msg_send_addr_clear_request (g_sock, c_yaddr[I], OTHER, D_INVALID(id, state));
    }
    else {
        joza_msg_send_addr_clear_request (g_sock, c_xaddr[I], OTHER, D_INVALID(id, state));
        joza_msg_send_addr_clear_request (g_sock, c_yaddr[I], SELF, D_INVALID(id, state));
    }

    // Unlike CLEARs requested by workers, a broker-initiated CLEAR
    // closes the channel immediately.
    remove_channel_by_idx(I);
}

// X is doing a hard stop.  We send a CLEAR_REQUEST to Y, close the
// channel immediately, and disconnect X.
void do_x_disconnect(joza_msg_t *M, int I)
{
    uint32_t hash;

    assert(R == X_CALLER);
    joza_msg_send_addr_clear_request (g_sock, c_yaddr[I], OTHER, D_WORKER_REQUESTED);

    hash = addr2hash(c_yaddr[I]);
    remove_channel_by_idx(I);
    remove_worker_by_hash(hash);
}

// X is closing down gracefully.
void do_x_clear_request(joza_msg_t *M, int I)
{
    diag_t d;
    // The worker's clear request shall only use the diagnostic D_WORKER_REQUESTED.
    d = joza_msg_diagnostic(M);
    if (d != D_WORKER_REQUESTED) {
        joza_msg_send_addr_clear_request (g_sock, c_xaddr[I], SELF, D_IMPROPER_DIAGNOSTIC_FROM_WORKER);
        joza_msg_send_addr_clear_request (g_sock, c_yaddr[I], OTHER, D_IMPROPER_DIAGNOSTIC_FROM_WORKER);
        remove_channel_by_idx(I);
    }
    else {
        joza_msg_send_addr_clear_request (g_sock, c_yaddr[I], OTHER, D_WORKER_REQUESTED);
        reset_channel_flow_by_idx[I];
        c_state[I] = state_x_clear_request;
    }
}

void do_x_clear_confirmation(joza_msg_t *M, int I)
{
    joza_msg_send_addr_clear_confirmation (g_sock, c_yaddr[I]);
    remove_channel_by_idx(I);
}


// To do this comparison, we use double-wide integer types to avoid worrying about
// numerical overflow.
bool_t flow_sequence_in_range(seq_t x, seq_t lo, seq_t hi)
{
    if (hi < lo) {
        if (x <= hi || x >= lo)
            return TRUE;
    }
    else if (hi > lo) {
        if (x >= lo && x <= hi)
            return TRUE;
    }
    return FALSE;
}


void do_x_data(joza_msg_t *M, int I)
{
    seq_t pr = joza_msg_pr(M);
    seq_t ps = joza_msg_ps(M);
    uint8_t  *data       = zframe_data(joza_msg_data(M));
    size_t   data_len    = zframe_size(joza_msg_data(M));

    // First, check if the message is valid

    // When X sends a message, its packet number should increases by 1 and should be 
    // in the window of messages that Y will allow
    if ((ps != w_xps[I])
        || !flow_sequence_in_range(ps, c_ypr[I], (c_ypr[I] + c_window[I]) % SEQ_MAX)) {
        joza_msg_send_addr_reset_request (g_sock, c_xaddr[I], SELF, D_INVALID_PS);
        joza_msg_send_addr_reset_request (g_sock, c_yaddr[I], OTHER, D_INVALID_PS);
        reset_channel_flow_by_idx[I];
        c_state[I] = state_y_reset_request;
    }
    // When X updates its own window of packets that it will accept,
    // its new lowest packet number that X will allow should be
    // between the previous lowest packet number that X would allow
    // and the next packet number that will be sent by Y.
    else if (!flow_sequence_in_range(pr, c_xpr[I], c_yps[I])) {
        joza_msg_send_addr_reset_request (g_sock, c_xaddr[I], SELF, D_INVALID_PR);
        joza_msg_send_addr_reset_request (g_sock, c_yaddr[I], OTHER, D_INVALID_PR);
        reset_channel_flow_by_idx[I];
        c_state[I] = state_y_reset_request;
    }
    else if (data_len == 0) {
        joza_msg_send_addr_reset_request (g_sock, c_xaddr[I], SELF, D_DATA_TOO_SMALL);
        joza_msg_send_addr_reset_request (g_sock, c_yaddr[I], OTHER, D_DATA_TOO_SMALL);
        reset_channel_flow_by_idx[I];
        c_state[I] = state_y_reset_request;        
    }
    else if (data_len > packet_bytes(c_tput[I])) {
        joza_msg_send_addr_reset_request (g_sock, c_xaddr[I], SELF, D_DATA_TOO_LARGE);
        joza_msg_send_addr_reset_request (g_sock, c_yaddr[I], OTHER, D_DATA_TOO_LARGE);
        reset_channel_flow_by_idx[I];
        c_state[I] = state_y_reset_request;
    }
    else
        joza_msg_send_addr_data (g_sock, c_yaddr[I], joza_msg_q(M), pr, ps, data);
}

static Channel
do_reset(Channel c, diagnostic_t d) {
    msg_t *x_msg = msg_new(MSG_RESET_REQUEST);
    msg_set_diagnostic(x_msg, d);
    connection_msg_send(&connection_store, c.x_key, &x_msg);

    msg_t *y_msg = msg_new(MSG_RESET_REQUEST);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(&connection_store, c.y_key, &y_msg);

    c.flow = reset(c.flow);
    c.state = state_y_reset_request;
    return c;
}

static Channel
do_x_call_collision(Channel c) {
    c.state = state_call_collision;
    return c;
}

static Channel
do_x_clear_confirmation(Channel c, uint8_t d) {
    // Y originally request a clear.  It was forwarded to X.  X has responded.
    // Forward the confirmation to Y
    msg_t *y_msg = msg_new(MSG_CLEAR_CONFIRMATION);
    msg_set_diagnostic(y_msg, d);
    connection_msg_send(&connection_store, c.y_key, &y_msg);

    // Clear this connection

    c.state = state_ready;
    return c;
}


#undef PUSH
