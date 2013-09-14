#include <assert.h>
#include <limits.h:
#include <stdint.h>
#include "lib.h"

#ifndef CHANNEL_COUNT
# define CHANNEL_COUNT UKEY_C(256)
#endif
static_assert(CHANNEL_COUNT >= UKEY_MAX, "CHANNEL_COUNT too large for ukey_t");

/*
lcn    lcn_t       [primary key]   the unique id for this connection
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
atime double                       time of last message from either peer
mtime double                       time of last modification to lcn/xaddr/yaddr/state/?p?/tput
*/                                                                


static size_t _count = 0;
static void *c_xzaddr[CHANNEL_COUNT]; /* ZMQ address of caller X */
static void *c_yzaddr[CHANNEL_COUNT]; /* ZMQ address of callee Y */
static size_t c_yidx[CHANNEL_COUNT]; /* index array that sorts ykey array */
static state_t c_state[CHANNEL_COUNT];
static seq_t c_xps[CHANNEL_COUNT]; /* sequence number of packets sent by X */
static seq_t c_xpr[CHANNEL_COUNT]; /* lowest packet sequence permitted to send to X */
static seq_t c_yps[CHANNEL_COUNT]; /* sequence number of packets sent by Y  */
static seq_t c_ypr[CHANNEL_COUNT]; /* lowest packet sequence permitted to send to Y */
static seq_t c_window[CHANNEL_COUNT];
static tput_t c_tput[CHANNEL_COUNT]; /* bits/sec permitted on this channel */

#define PUSH(arr,idx,count)                                             \
    memmove(arr + idx + 1, arr + idx, sizeof(arr[0]) * (count - idx))

lcn_t add_channel(zframe_t *xzaddr, zframe_t *yzaddr)
{
    assert(_count < CHANNEL_COUNT);
    assert(xkey < UKEY_MAX);
    assert(ykey < UKEY_MAX);

    size_t idx;

    idx = keyfind(c_xkey, _count, xkey);
    assert (c_xkey[idx] != xkey);

    if (idx < _count)
    {
        PUSH(c_xkey, idx, _count);
        PUSH(c_ykey, idx, _count);
        PUSH(c_xzaddr, idx, _count);
        PUSH(c_yzaddr, idx, _count);
        PUSH(c_state, idx, _count);
        PUSH(c_xps, idx, _count);
        PUSH(c_yps, idx, _count);
        PUSH(c_xpr, idx, _count);
        PUSH(c_ypr, idx, _count);
        PUSH(c_window, idx, _count);
        PUSH(c_tput, idx, _count);
    }
    c_xkey[idx] = xkey;
    c_ykey[idx] = ykey;
    c_xzaddr[idx] = xzaddr;
    c_yzaddr[idx] = yzaddr;
    c_state[idx] = READY;
    c_xps[idx] = SEQ_MIN;
    c_xps[idx] = SEQ_MIN;
    c_ypr[idx] = SEQ_MIN;
    c_ypr[idx] = SEQ_MIN;
    c_window[idx] = WINDOW_DEFAULT;
    c_tput[idx] = TPUT_DEFAULT;

    _count ++;

    indexx(c_ykey, _count, c_yidx);
};    

find_channel(key_t 

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
