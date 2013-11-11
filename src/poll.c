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

#include <glib.h>
#include <czmq.h>

#include "poll.h"

#include "worker.h"
#include "channel.h"
#include "msg.h"

#include "joza_msg.h"

// Zero MQ socket
static zmq_pollitem_t poll_input = {NULL, 0, ZMQ_POLLIN, 0};

static int s_recv_(zloop_t *loop, zmq_pollitem_t *item, void *arg);
static int s_process_(joza_msg_t *msg, void *sock);

static zctx_t *zctx_new_or_die (void)
{
    zctx_t *c = NULL;

    c = zctx_new();
    if (c == NULL) {
        g_print("failed to create a ZeroMQ context");
        exit(1);
    }
    return c;
}

static void *zsocket_new_or_die(zctx_t *ctx, int type)
{
    void *sock;

    g_assert(ctx != NULL);

    sock = zsocket_new(ctx, type);
    if (sock == NULL) {
        g_print("failed to create a new ZeroMQ socket");
        exit(1);
    }
    return sock;
}

static zloop_t *zloop_new_or_die(void)
{
    zloop_t *L = NULL;

    L = zloop_new();
    if (L == NULL) {
        g_error("failed to create a new ZeroMQ main loop");
        exit (1);
    }
    return L;
}


joza_poll_t *poll_create(gboolean verbose, const char *endpoint)
{
    g_message("In %s(verbose = %d, endpoint = %s)", __FUNCTION__, verbose, endpoint);

    joza_poll_t *poll = g_new0(joza_poll_t, 1);

    //  Initialize broker state
    poll->ctx = zctx_new_or_die();
    poll->sock = zsocket_new_or_die(poll->ctx, ZMQ_ROUTER);
    g_message("binding ROUTER socket to %s", endpoint);
    zsocket_bind(poll->sock, endpoint);
    poll->loop = zloop_new_or_die();
    zloop_set_verbose(poll->loop, verbose);
    poll_input.socket = poll->sock;
    int rc = zloop_poller(poll->loop, &poll_input, s_recv_, poll->sock);
    if (rc == -1) {
        g_error("failed to start a new ZeroMQ main loop poller");
        exit(1);
    }
    return poll;
}

// This is the main callback that gets called whenever a
// message is received from the ZeroMQ loop.
static int s_recv_(zloop_t *loop, zmq_pollitem_t *item, void *sock)
{
    joza_msg_t *msg = NULL;
    int ret = 0;

    g_message("In %s(%p,%p,%p)", __FUNCTION__, loop, item, sock);

    if (item->revents & ZMQ_POLLIN) {
        msg = joza_msg_recv(sock);
        if (msg != NULL) {
            // Process the valid message
            ret = s_process_(msg, sock);
            joza_msg_destroy(&msg);
        }
    }

    g_message("%s(%p,%p,%p) returns %d", __FUNCTION__, loop, item, sock, ret);
    return ret;
}

// This is entry point for message processing.  Every message
// being processed start here.
static int s_process_(joza_msg_t *msg, void *sock)
{
    wkey_t key;
    bool_index_t bi_worker;
    gboolean more = FALSE;
    role_t role = READY;
    int ret = 0;

    g_message("In %s(%p)", __FUNCTION__, msg);

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
            channel_dispatch_by_lcn(sock, msg, w_lcn[bi_worker.index], 0);
        else
            channel_dispatch_by_lcn(sock, msg, w_lcn[bi_worker.index], 1);
    }

    // If this worker is connected, but, not part of a call, the
    // connection handler handles the message.
    else if (bi_worker.flag == TRUE) {
        more = worker_dispatch_by_idx(sock, msg, bi_worker.index);
        if (more)
            goto do_more;
    }

    // If this worker is heretofore unknown, we handle it here.  The
    // only message we handle from unconnected workers are connection
    // requests.
    else {
        const char *cmdname = joza_msg_const_command(msg);

        if (joza_msg_id(msg) == JOZA_MSG_CONNECT) {
            g_message("handling %s from unconnected worker", cmdname);
            worker_add(sock,
                       joza_msg_const_address(msg),
                       joza_msg_const_calling_address(msg),
                       (iodir_t) joza_msg_const_directionality(msg));
        } 
        else if (joza_msg_id(msg) == JOZA_MSG_DIRECTORY_REQUEST) {
            g_message("sending directory");

            zhash_t *dir = worker_directory();
            joza_msg_send_addr_directory (sock,
                                          joza_msg_const_address(msg),
                                          dir);
            zhash_destroy(&dir);
        }
        else {
            g_message("ignoring %s from unconnected worker", cmdname);
        }
    }

    g_message("%s(%p) returns %d", __FUNCTION__, msg, ret);
    return ret;
}


void poll_start(zloop_t *loop)
{
    // Takes over control of the thread.
    g_message("In %s()", __FUNCTION__);

    g_message("starting main loop");
    zloop_start(loop);
}

void poll_destroy(joza_poll_t *poll)
{
    zloop_destroy(&(poll->loop));
    zsocket_destroy(poll->ctx, poll->sock);
    zctx_destroy(&(poll->ctx));
    g_free(poll);
}

