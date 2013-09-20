/*
    poll.c - event loop

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

#include <czmq.h>

#include "poll.h"

#include "bool.h"
#include "log.h"
#include "worker.h"
#include "channel.h"
#include "msg.h"

#include "joza_msg.h"

// Zero MQ socket
void *g_poll_sock;

static zctx_t *ctx;
static zloop_t *loop;
static zmq_pollitem_t poll_input = {NULL, 0, ZMQ_POLLIN, 0};

static int s_recv_(zloop_t *loop, zmq_pollitem_t *item, void *arg);
static int s_process_(joza_msg_t *msg);

static zctx_t *zctx_new_or_die (void)
{
    zctx_t *c = NULL;

    TRACE("In %s()", __FUNCTION__);

    c = zctx_new();
    if (c == NULL) {
        ERR("failed to create a ZeroMQ context");
        exit(1);
    }
    return c;
}

static void *zsocket_new_or_die(zctx_t *ctx, int type)
{
    void *sock;

    TRACE("In %s(ctx = %p, type = %d)", __FUNCTION__, ctx, type);
    assert(ctx != NULL);

    sock = zsocket_new(ctx, type);
    if (sock == NULL) {
        ERR("failed to create a new ZeroMQ socket");
        exit(1);
    }
    return sock;
}

static zloop_t *zloop_new_or_die(void)
{
    zloop_t *L = NULL;

    TRACE("In %s()", __FUNCTION__);

    L = zloop_new();
    if (L == NULL) {
        ERR("failed to create a new ZeroMQ main loop");
        exit (1);
    }
    return L;
}


void poll_init(bool_t verbose, const char *endpoint)
{
    TRACE("In %s(verbose = %d, endpoint = %s)", __FUNCTION__, verbose, endpoint);

    //  Initialize broker state
    ctx = zctx_new_or_die();
    g_poll_sock = zsocket_new_or_die(ctx, ZMQ_ROUTER);
    NOTE("binding ROUTER socket to %s", endpoint);
    zsocket_bind(g_poll_sock, endpoint);
    loop = zloop_new_or_die();
    zloop_set_verbose(loop, verbose);
    poll_input.socket = g_poll_sock;
    int rc = zloop_poller(loop, &poll_input, s_recv_, NULL);
    if (rc == -1) {
        ERR("failed to start a new ZeroMQ main loop poller");
        exit(1);
    }
}

// This is the main callback that gets called whenever a
// message is received from the ZeroMQ loop.
static int s_recv_(zloop_t *loop, zmq_pollitem_t *item, void *arg)
{
    joza_msg_t *msg = NULL;
    int ret = 0;

    TRACE("In %s(%p,%p,%p)", __FUNCTION__, loop, item, arg);

    if (item->revents & ZMQ_POLLIN) {
        NOTE("Socket has POLLIN");
        msg = joza_msg_recv(g_poll_sock);
        if (msg != NULL) {
            // Process the valid message
            ret = s_process_(msg);
        }
    }

    TRACE("%s(%p,%p,%p) returns %d", __FUNCTION__, loop, item, arg, ret);
    return ret;
}

// This is entry point for message processing.  Every message
// being processed start here.
static int s_process_(joza_msg_t *msg)
{
    wkey_t key;
    bool_index_t bi_worker;
    bool_t more = FALSE;
    role_t role = READY;
    int ret = 0;

    TRACE("In %s(%p)", __FUNCTION__, msg);

    key = msg_addr2key(joza_msg_address(msg));
do_more:

    bi_worker = worker_get_idx_by_key(key);
    if (bi_worker.flag == TRUE) {
        role = w_role[bi_worker.index];
    }

    // If this worker is connected and part of a virtual call, the
    // call's state machine processes the message.
    if (role == X_CALLER || role == Y_CALLEE) {
        if (role == X_CALLER)
            channel_dispatch_by_lcn(msg, w_lcn[bi_worker.index], 0);
        else
            channel_dispatch_by_lcn(msg, w_lcn[bi_worker.index], 1);
    }

    // If this worker is connected, but, not part of a call, the
    // connection handler handles the message.
    else if (bi_worker.flag == TRUE) {
        more = worker_dispatch_by_idx(msg, bi_worker.index);
        if (more)
            goto do_more;
    }

    // If this worker is heretofore unknown, we handle it here.  The
    // only message we handle from unconnected workers are connection
    // requests.
    else {
        const char *cmdname = joza_msg_const_command(msg);

        if (joza_msg_id(msg) == JOZA_MSG_CONNECT) {
            INFO("handling %s from unconnected worker", cmdname);
            worker_add(joza_msg_const_address(msg),
                       joza_msg_const_calling_address(msg),
                       (iodir_t) joza_msg_const_directionality(msg));
        } else {
            INFO("ignoring %s from unconnected worker", cmdname);
        }
    }

    TRACE("%s(%p) returns %d", __FUNCTION__, msg, ret);
    return ret;
}


void poll_start(void)
{
    // Takes over control of the thread.
    TRACE("In %s()", __FUNCTION__);

    NOTE("starting main loop");
    zloop_start(loop);
}

