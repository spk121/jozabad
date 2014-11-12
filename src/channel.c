/*
    channel.c - peer-to-peer connections

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
#include <string.h>
#include <czmq.h>

#include "joza_msg.h"
#include "action.h"
#include "cause.h"
#include "channel.h"
#include "diag.h"
#include "lib.h"
#include "poll.h"
#include "state.h"
#include "tput.h"
#include "worker.h"
#include "packet.h"
#include "msg.h"
#include "packet.h"

// #include "../libjoza/joza_lib.h"

// static_assert(ACTION_STATE_COUNT == state_last + 1, "Number of state different than action table");
// static_assert(ACTION_MESSAGE_COUNT == JOZA_MSG_COUNT, "Number of messages different than action table");
static_assert(sizeof(seq_t) <= offsetof(joza_msg_t,ps) - offsetof(joza_msg_t,pr), "Sequence type too large");

#define STATE2DIAG(s) ((diag_t)((s) - state_ready + d_invalid_message_for_state_ready))


channel_t *
channel_create(lcn_t lcn, zframe_t *xzaddr, const char *xname, zframe_t *yzaddr, const char *yname,
               packet_t pkt, seq_t window, tput_t tput)
{
    channel_t *c;

    c = g_new0(channel_t, 1);

    c->lcn = lcn;
    c->xzaddr = zframe_dup(xzaddr);
    c->yzaddr = zframe_dup(yzaddr);
    c->xname = g_strdup(xname);
    c->yname = g_strdup(yname);
    c->state = state_ready;
    c->xps = SEQ_MIN;
    c->xpr = SEQ_MIN;
    c->yps = SEQ_MIN;
    c->ypr = SEQ_MIN;
    c->pkt = pkt;
    c->window = window;
    c->tput = tput;
    c->ctime = g_get_monotonic_time();
    c->mtime = g_get_monotonic_time();

    g_message("creating channel #%d %s/%s - packet = %d, window = %d, tput %d", c->lcn, xname, yname, packet_bytes(pkt),
              window, tput_bps(tput));
    g_debug("%s(xzaddr = %p, xname = %s, yzaddr = %p, yname = %s, pkt = %d, window = %d, tput = %d) returns %p",
              __FUNCTION__, (void *) xzaddr, xname, (void *) yzaddr, yname, pkt, window, tput, (void *)c);

    return c;
}

// This punishment action is a result of a message received a worker
// that is incorrect for the current state.
static state_t do_reset(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, state_t state)
__attribute__ ((warn_unused_result));

static state_t do_reset(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, state_t state)
{
    joza_msg_send_addr_reset_request(sock, self_zaddr, c_local_procedure_error, STATE2DIAG(state));
    joza_msg_send_addr_reset_request(sock, other_zaddr, c_remote_procedure_error, STATE2DIAG(state));

    return state_y_reset_request;
}

// This punishement action is a result of a message received from a
// worker that is incorrect for the current state.
static state_t do_clear(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, state_t state)
__attribute__ ((warn_unused_result));

static state_t do_clear(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, state_t state)
{
    joza_msg_send_addr_clear_request(sock, self_zaddr, c_local_procedure_error, STATE2DIAG(state));
    joza_msg_send_addr_clear_request(sock, other_zaddr, c_remote_procedure_error, STATE2DIAG(state));
    // Unlike CLEARs requested by workers, a broker-initiated CLEAR
    // closes the channel immediately.
    return state_ready;
}

// Caller is doing a hard stop. I send a CLEAR_REQUEST to callee,
// close the channel immediately, and disconnect caller
static state_t do_i_disconnect(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr)
__attribute__ ((warn_unused_result));

static state_t do_i_disconnect(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr)
{
    int ret;
    ret = joza_msg_send_addr_clear_request(sock, other_zaddr, c_worker_originated, d_worker_originated);
    if (ret == -1)
        diagnostic(sock, self_zaddr, NULL, c_zmq_sendmsg_err, errno2diag());
    return state_ready;
}

static state_t do_y_call_accepted(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, joza_msg_t *M, gchar *self_name, gchar *other_name,
                                  state_t state, seq_t *self_window, packet_t *self_pkt, tput_t *self_tput)
{
    char     *xname      = joza_msg_calling_address(M);
    char     *yname      = joza_msg_called_address(M);
    packet_t pkt         = (packet_t) joza_msg_packet(M);
    tput_t   tput        = (tput_t) joza_msg_throughput(M);
    seq_t    window      = joza_msg_window(M);

    // If the caller has modified this channel's facilities,
    // ensure that the caller has obey the negotiation rules
    if (!packet_negotiate(pkt, *self_pkt))
        diagnostic(sock, self_zaddr, self_name, c_invalid_facility_request, d_invalid_packet_facility_negotiation);
    else if (!window_negotiate(window, *self_window))
        diagnostic(sock, self_zaddr, self_name, c_invalid_facility_request, d_invalid_window_facility_negotiation);
    else if (!tput_negotiate(tput, *self_tput))
        diagnostic(sock, self_zaddr, self_name, c_invalid_facility_request, d_invalid_throughput_facility_negotiation);

    // TODO: Validate that the addresses still match.
    // If the callee's address has changed, this worker is requesting that the broker forward
    // this message to another worker for processing.  If the caller's address has changed, well,
    // I don't know what that means, yet.
    else if(strcmp(xname, other_name) != 0)
        diagnostic(sock, self_zaddr, self_name, c_invalid_forwarding_request, d_caller_forwarding_not_allowed);
    else if(strcmp(yname, self_name) != 0)
        diagnostic(sock, self_zaddr, self_name, c_invalid_forwarding_request, d_callee_forwarding_not_allowed);

    else {

        // Update the channel
        *self_pkt = pkt;
        *self_window = window;
        *self_tput = tput;

        joza_msg_send_addr_call_accepted (sock, other_zaddr, xname, yname,
                                          pkt, window, tput, joza_msg_data(M));
        return state_data_transfer;
    }
    return state;
}

// CALL COLLISION -- To get here, X has sent a call request, and the
// broker has joined X and Y in a call.  X is waiting for a
// CALL_ACCEPTED, but, Y has sent a CALL_REQUEST instead, probably
// because X and Y tried to join at the same time.
//
// To resolve this, broker informs Y of the call collision.  Y is
// supposed to then send a CALL_ACCEPTED.
static state_t do_y_call_collision(void *sock, zframe_t *self_zaddr)
__attribute__ ((warn_unused_result));

static state_t do_y_call_collision(void *sock, zframe_t *self_zaddr)
{
    diagnostic(sock, self_zaddr, NULL, c_call_collision, d_call_collision);
    return state_call_collision;
}

// Caller is closing down gracefully
static state_t do_i_clear_request(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, joza_msg_t *M, gboolean is_y, state_t state)
{
    // The caller's clear request shall only use the diagnostic D_WORKER_REQUESTED.
    if (joza_msg_cause(M) != c_worker_originated)
        diagnostic(sock, self_zaddr, NULL, c_malformed_message, d_invalid_cause);
    else if (joza_msg_diagnostic(M) != d_worker_originated)
        diagnostic(sock, self_zaddr, NULL, c_malformed_message, d_invalid_diagnostic);
    else {
        joza_msg_send_addr_clear_request(sock, other_zaddr, c_worker_originated, d_worker_originated);
        return STATE_CLEAR_REQUEST(is_y);
    }
    return state;
}

// Caller is responding to a peer's request to close the channel
static state_t do_i_clear_confirmation(void *sock, zframe_t *other_zaddr, seq_t *xps, seq_t *xpr, seq_t *yps, seq_t *ypr)
{
    joza_msg_send_addr_clear_confirmation (sock, other_zaddr);
    *xps = 0;
    *xpr = 0;
    *yps = 0;
    *ypr = 0;
    return state_ready;
}

static void do_i_data(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, joza_msg_t *M, packet_t packet_size, seq_t window_size, seq_t *self_ps, seq_t *self_pr, seq_t other_ps, seq_t other_pr)
{
    seq_t pr = joza_msg_pr(M);
    seq_t ps = joza_msg_ps(M);
    size_t   data_len    = zframe_size(joza_msg_data(M));

    // First, check if the message is valid
    if (data_len > packet_bytes(packet_size))
        diagnostic(sock, self_zaddr, NULL, c_local_procedure_error, d_data_too_long_for_packet_facility);

    // When caller sends a message, its packet number should match my
    // expected packet number for this caller, and should be in the
    // window of packet numbers that callee has said it will accept.
    else if (ps != *self_ps)
        diagnostic(sock, self_zaddr, NULL, c_local_procedure_error, d_ps_out_of_order);
    else if (!seq_in_range(ps, other_pr, (other_pr + window_size - 1) % SEQ_MAX))
        diagnostic(sock, self_zaddr, NULL, c_local_procedure_error, d_ps_not_in_window);
    // When X updates its own window of packets that it will accept,
    // its new lowest packet number that X will allow should be
    // between the previous lowest packet number that X would allow
    // and the next packet number that will be sent by Y.
    else if (!seq_in_range(pr, *self_pr, other_ps))
        diagnostic(sock, self_zaddr, NULL, c_local_procedure_error, d_pr_invalid_window_update);
    else {
        joza_msg_send_addr_data (sock, other_zaddr, joza_msg_q(M), pr, ps, joza_msg_data(M));
        *self_ps = (*self_ps + 1) % (SEQ_MAX + 1);
        *self_pr = pr;
    }
}


// Caller tells callee that it is updating the range of packet numbers
// it will allow.
static void do_i_rr(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, joza_msg_t *M, seq_t *self_pr, seq_t other_ps)
{
    seq_t pr = joza_msg_pr(M);

    if (pr > SEQ_MAX)
        diagnostic(sock, self_zaddr, NULL, c_malformed_message, d_pr_too_large);
    else if (!seq_in_range(pr, *self_pr, other_ps))
        diagnostic(sock, self_zaddr, NULL, c_local_procedure_error, d_pr_invalid_window_update);
    else {
        joza_msg_send_addr_rr (sock, other_zaddr, pr);
        *self_pr = pr;
    }
}


// Caller tells callee that it is updating the range of packet numbers
// it will allow, and that it should stop sending data as soon as possible.
static void do_i_rnr(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, joza_msg_t *M, seq_t *self_pr, seq_t other_ps)
{
    seq_t pr = joza_msg_pr(M);

    // First, check if the message is valid

    if (pr > SEQ_MAX)
        diagnostic(sock, self_zaddr, NULL, c_malformed_message, d_pr_too_large);
    else if (!seq_in_range(pr, *self_pr, other_ps))
        diagnostic(sock, self_zaddr, NULL, c_local_procedure_error, d_pr_invalid_window_update);
    else {
        joza_msg_send_addr_rnr (sock, other_zaddr, pr);
        *self_pr = pr;
    }
}


// Caller is requesting that callee reset flow control
static void do_i_reset(void *sock, zframe_t *self_zaddr, zframe_t *other_zaddr, joza_msg_t *M)
{
    if (joza_msg_cause(M) != c_worker_originated)
        diagnostic(sock, self_zaddr, NULL, c_malformed_message, d_invalid_cause);
    else if (joza_msg_diagnostic(M) != d_worker_originated)
        diagnostic(sock, self_zaddr, NULL, c_malformed_message, d_invalid_diagnostic);
    else {
        joza_msg_send_addr_reset_request (sock, other_zaddr, c_worker_originated, d_worker_originated);
    }
}

// Caller has confirmed callee's request for a reset.
static state_t do_i_reset_confirmation(void *sock, zframe_t *other_zaddr, seq_t *self_ps, seq_t *self_pr, seq_t *other_ps, seq_t *other_pr)
__attribute__ ((warn_unused_result));

static state_t do_i_reset_confirmation(void *sock, zframe_t *other_zaddr, seq_t *self_ps, seq_t *self_pr, seq_t *other_ps, seq_t *other_pr)
{
    // The worker's reset request shall only use the diagnostic D_WORKER_REQUESTED.
    joza_msg_send_addr_reset_confirmation (sock, other_zaddr);
    *self_ps = 0;
    *self_pr = 0;
    *other_ps = 0;
    *other_pr = 0;
    return state_data_transfer;
}

state_t channel_dispatch(channel_t *channel, void *sock, joza_msg_t *M, bool is_y)
{
    // g_message("In %s(%p,%d,%d)", __FUNCTION__, channel, (void *)sock, M, is_y);

    channel->mtime = g_get_monotonic_time();

    state_t state = channel->state;
    state_t state_orig = channel->state;
    action_t a = action_get(state, joza_msg_const_id(M), is_y);
    gchar *self_name = is_y ? channel->yname : channel->xname;
    zframe_t *self_zaddr = is_y ? channel->yzaddr : channel->xzaddr;
    zframe_t *other_zaddr = !is_y ? channel->yzaddr : channel->xzaddr;

    g_message("%s/%s received %s from %s in state %s and dispatched %s",
              channel->xname,
              channel->yname,
              joza_msg_const_command(M),
              self_name,
              state_name(state),
              action_name(a));

    switch (a) {
    case a_unspecified:
    case a_disconnect:
    case a_x_connect:
    case a_x_call_request:
    case a_x_call_accepted:
    case a_y_call_request:
        // None of these should ever happen.  X is always connected
        // before Y. And this channel is already set-up by the time
        // this function is called.
        abort();
        break;

    case a_discard:
        break;

    case a_reset:
        state = do_reset(sock, self_zaddr, other_zaddr, state);
        break;
    case a_clear:
        state = do_clear(sock, self_zaddr, other_zaddr, state);
        break;

    case a_x_disconnect:
        state = do_i_disconnect(sock, self_zaddr, other_zaddr);
        break;
    case a_x_clear_request:
        state = do_i_clear_request(sock, self_zaddr, other_zaddr, M, 0, state);
        break;
    case a_x_clear_confirmation:
        state = do_i_clear_confirmation(sock, other_zaddr, &channel->xps, &channel->xpr, &channel->yps, &channel->ypr);
        break;
    case a_x_data:
        do_i_data(sock, self_zaddr, other_zaddr, M, channel->pkt, channel->window, &channel->xps, &channel->xpr, channel->yps, channel->ypr);
        break;
    case a_x_rr:
        do_i_rr(sock, self_zaddr, other_zaddr, M, &channel->xpr, channel->yps);
        break;
    case a_x_rnr:
        do_i_rnr(sock, self_zaddr, other_zaddr, M, &channel->xpr, channel->yps);
        break;
    case a_x_reset:
        do_i_reset(sock, self_zaddr, other_zaddr, M);
        break;
    case a_x_reset_confirmation:
        state = do_i_reset_confirmation(sock, other_zaddr, &channel->xps, &channel->xpr,
                                        &channel->yps, &channel->ypr);
        break;

    case a_y_disconnect:
        state = do_i_disconnect(sock, self_zaddr, other_zaddr);
        break;
    case a_y_call_accepted:
        state = do_y_call_accepted(sock, self_zaddr, other_zaddr, M, channel->yname, channel->xname,
                                   state, &channel->window, &channel->pkt, &channel->tput);
        break;
    case a_y_call_collision:
        state = do_y_call_collision(sock, self_zaddr);
        break;
    case a_y_clear_request:
        state = do_i_clear_request(sock, self_zaddr, other_zaddr, M, 1, state);
        break;
    case a_y_clear_confirmation:
        state = do_i_clear_confirmation(sock, other_zaddr, &channel->xps, &channel->xpr, &channel->yps, &channel->ypr);
        break;
    case a_y_data:
        do_i_data(sock, self_zaddr, other_zaddr, M, channel->pkt, channel->window, &channel->yps, &channel->ypr, channel->xps, channel->xpr);
        break;
    case a_y_rr:
        do_i_rr(sock, self_zaddr, other_zaddr, M, &channel->ypr, channel->xps);
        break;
    case a_y_rnr:
        do_i_rnr(sock, self_zaddr, other_zaddr, M, &channel->ypr, channel->xps);
        break;
    case a_y_reset:
        do_i_reset(sock, self_zaddr, other_zaddr, M);
        break;
    case a_y_reset_confirmation:
        state = do_i_reset_confirmation(sock, other_zaddr, &channel->yps, &channel->ypr,
                                        &channel->xps, &channel->xpr);
        break;
    }

    if (state_orig != state)
        g_message("%s/%s after %s state is now %s", channel->xname, channel->yname,
                  action_name(a), state_name(state));
    return state;
}

void channel_set_state(channel_t *channel, state_t state)
{
    channel->state = state;
}

gboolean channel_is_closed(channel_t *channel)
{
    return channel->state == state_ready;
}
